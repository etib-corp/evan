/*
** ETIB PROJECT, 2026
** evan
** File description:
** ObjectRegistry
*/

#pragma once

#include <cstddef>
#include <cstdint>
#include <memory>
#include <utility>
#include <vector>

namespace evan
{
	/**
	 * @brief Dense slot-map registry with stable, generational handles.
	 *
	 * Objects live in a contiguous vector of slots; freed slots are recycled
	 * through a free list and their generation is bumped, so every handle
	 * handed out for a previous occupant of the slot stays invalid forever.
	 * Iteration walks the contiguous storage, which is the access pattern of
	 * the per-frame draw-list build (see `docs/DESIGN_P3_OPTIMIZATIONS.md`,
	 * workstream WS2).
	 *
	 * A handle is an opaque `std::size_t` packing the slot generation in the
	 * high 32 bits and the slot index in the low 32 bits. Handle 0 is never
	 * produced, so callers can keep using 0 as an "invalid" sentinel.
	 * Packed handles require a 64-bit `std::size_t` (see the static assert
	 * below); every current target (macOS, Linux, Windows and Android
	 * arm64-v8a) satisfies it.
	 *
	 * @tparam T Type of the registered objects.
	 */
	template<typename T> class ObjectRegistry
	{
		public:
		/**
		 * @brief Handle identifying one registered object.
		 */
		using Handle = std::size_t;

		/**
		 * @brief Handle value that never identifies an object.
		 */
		static constexpr Handle kInvalidHandle = 0;

		/**
		 * @brief Registers @p object and returns its handle.
		 *
		 * A vacant slot is recycled when one exists, otherwise a new slot is
		 * appended. The handle of a recycled slot differs from the handle of
		 * its previous occupant.
		 *
		 * @param object The object to register. Must not be null.
		 * @return The handle of the registered object, or kInvalidHandle
		 * when @p object is null.
		 */
		[[nodiscard]] Handle add(std::shared_ptr<T> object)
		{
			if (object == nullptr) {
				return kInvalidHandle;
			}

			std::size_t index = 0;
			if (!_freeList.empty()) {
				index = _freeList.back();
				_freeList.pop_back();
				_slots[index].object = std::move(object);
			} else {
				index = _slots.size();
				_slots.push_back(Slot { 1, std::move(object) });
			}
			++_size;
			return makeHandle(_slots[index].generation,
							  static_cast<std::uint32_t>(index));
		}

		/**
		 * @brief Resolves @p handle to the registered object.
		 *
		 * @param handle The handle returned by add().
		 * @return The registered object, or nullptr when the handle is
		 * unknown or stale (its object was removed).
		 */
		[[nodiscard]] std::shared_ptr<T> get(Handle handle) const
		{
			const Slot *slot = slotFor(handle);
			return slot != nullptr ? slot->object : nullptr;
		}

		/**
		 * @brief Replaces the object registered under @p handle.
		 *
		 * @param handle The handle returned by add().
		 * @param object The new object. Must not be null.
		 * @return True when the handle was valid and the object replaced,
		 * false otherwise.
		 */
		bool update(Handle handle, std::shared_ptr<T> object)
		{
			if (object == nullptr) {
				return false;
			}
			Slot *slot = slotFor(handle);
			if (slot == nullptr) {
				return false;
			}
			slot->object = std::move(object);
			return true;
		}

		/**
		 * @brief Unregisters the object behind @p handle.
		 *
		 * The slot is recycled and its generation bumped, so @p handle (and
		 * every copy of it) stays invalid forever.
		 *
		 * @param handle The handle returned by add().
		 * @return True when the handle was valid and the object removed,
		 * false otherwise.
		 */
		bool remove(Handle handle)
		{
			Slot *slot = slotFor(handle);
			if (slot == nullptr) {
				return false;
			}
			slot->object = nullptr;
			++slot->generation;
			_freeList.push_back(handleIndex(handle));
			--_size;
			return true;
		}

		/**
		 * @brief Number of registered objects.
		 *
		 * @return The number of live objects.
		 */
		[[nodiscard]] std::size_t size() const noexcept
		{
			return _size;
		}

		/**
		 * @brief Whether no object is registered.
		 *
		 * @return True when the registry is empty.
		 */
		[[nodiscard]] bool empty() const noexcept
		{
			return _size == 0;
		}

		/**
		 * @brief Drops every registered object.
		 *
		 * Every handle handed out before is invalid afterwards.
		 */
		void clear() noexcept
		{
			_slots.clear();
			_freeList.clear();
			_size = 0;
		}

		/**
		 * @brief Visits every live object in slot order.
		 *
		 * Slot order equals registration order until a slot is recycled: a
		 * recycled slot appears at its original position with its new
		 * object.
		 *
		 * @param visitor Callable invoked as visitor(handle, object) for
		 * each live object.
		 */
		template<typename Visitor> void forEach(Visitor &&visitor) const
		{
			for (std::size_t i = 0; i < _slots.size(); ++i) {
				const Slot &slot = _slots[i];
				if (slot.object == nullptr) {
					continue;
				}
				visitor(
					makeHandle(slot.generation, static_cast<std::uint32_t>(i)),
					slot.object);
			}
		}

		private:
		/**
		 * @brief One registry slot: the object and its generation.
		 */
		struct Slot {
			std::uint32_t generation = 1;	 ///< Bumped when the slot is freed.
			std::shared_ptr<T> object;		 ///< Null when the slot is vacant.
		};

		// Packed handles need a 64-bit std::size_t: 32 bits of slot index
		// and 32 bits of generation.
		static_assert(sizeof(std::size_t) >= sizeof(std::uint64_t),
					  "ObjectRegistry packed handles require a 64-bit "
					  "std::size_t");

		///< Mask of the slot index inside a packed handle.
		static constexpr std::uint32_t kIndexMask = 0xFFFFFFFFu;

		/**
		 * @brief Packs a slot generation and index into a handle.
		 */
		static Handle makeHandle(std::uint32_t generation,
								 std::uint32_t index) noexcept
		{
			return (static_cast<Handle>(generation) << 32)
				| static_cast<Handle>(index);
		}

		/**
		 * @brief Extracts the slot index of a packed handle.
		 */
		static std::uint32_t handleIndex(Handle handle) noexcept
		{
			return static_cast<std::uint32_t>(handle & kIndexMask);
		}

		/**
		 * @brief Extracts the slot generation of a packed handle.
		 */
		static std::uint32_t handleGeneration(Handle handle) noexcept
		{
			return static_cast<std::uint32_t>(handle >> 32);
		}

		/**
		 * @brief Resolves @p handle to its slot when it is live.
		 *
		 * @param handle The handle to resolve.
		 * @return The slot, or nullptr when the handle does not denote a
		 * live object.
		 */
		[[nodiscard]] const Slot *slotFor(Handle handle) const noexcept
		{
			const std::uint32_t index = handleIndex(handle);
			if (index >= _slots.size()) {
				return nullptr;
			}
			const Slot &slot = _slots[index];
			if (slot.object == nullptr
				|| slot.generation != handleGeneration(handle)) {
				return nullptr;
			}
			return &slot;
		}

		/**
		 * @brief Non-const overload of slotFor().
		 */
		[[nodiscard]] Slot *slotFor(Handle handle) noexcept
		{
			return const_cast<Slot *>(std::as_const(*this).slotFor(handle));
		}

		std::vector<Slot> _slots;				 ///< Dense storage, vacant
												 ///< slots included.
		std::vector<std::uint32_t> _freeList;	 ///< Indices of vacant slots.
		std::size_t _size = 0;					 ///< Number of live objects.
	};
}	 // namespace evan
