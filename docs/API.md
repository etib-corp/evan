# API

This document provides an overview of the API for our application for ensure a seamless and consistent user experience and behavior between the XR (extended reality) and desktop environments, particularly in terms of UI, data, and interactions.
Here’s a breakdown of the **goals and responsibilities** of each class in your API, followed by the code. This will help clarify the purpose and usage of each component in your cross-platform (XR/desktop) application.

## **1. Application**

**Goal**:
The `Application` class is the root of your application. It manages the global state, including the 3D `Scene3D` and all `UIPanel`s. Its role is to provide a high-level interface for creating, accessing, and managing UI panels and the 3D scene, ensuring consistency across XR and desktop environments.

**Responsibilities**:

- Own and manage the `Scene3D` (3D environment).
- Add, retrieve, and remove `UIPanel`s (2D surfaces).
- Serve as the entry point for the application lifecycle.

**Code**:

```cpp
/**
 * @class Application
 * @brief Root class that manages the global application state, including the 3D Scene and all UI panels.
 * 
 * The Application class serves as the entry point for the application lifecycle and provides
 * a high-level interface for creating, accessing, and managing UI panels and the 3D scene,
 * ensuring consistency across XR and desktop environments.
 */
class Application {
private:
    std::unique_ptr<Scene3D> _scene;                    ///< The 3D environment scene
    std::vector<std::unique_ptr<UIPanel>> _panels;      ///< Collection of 2D UI panels
public:
    virtual ~Application() = default;

    /**
     * @brief Gets a reference to the 3D scene.
     * @return Reference to the Scene3D.
     */
    Scene3D& getScene() const { return *_scene; }

    /**
     * @brief Adds a new UI panel to the application.
     * @param name The name identifier for the UI panel.
     */
    void addPanel(std::string_view name);
    
    /**
     * @brief Retrieves a UI panel by name.
     * @param name The name of the UI panel to retrieve.
     * @return Optional reference to the UIPanel if found.
     */
    std::optional<std::reference_wrapper<UIPanel>> getPanel(std::string_view name) const;
    
    /**
     * @brief Removes a UI panel by name.
     * @param name The name of the UI panel to remove.
     */
    void removePanel(std::string_view name);
    
    /**
     * @brief Gets all UI panels in the application.
     * @return Const reference to the vector of all UI panels.
     */
    const std::vector<std::unique_ptr<UIPanel>>& getPanels() const { return _panels; }
};
```

---

## **2. Scene3D**

**Goal**:
The `Scene3D` class represents a 3D environment where `Entity3D` objects (3D entities) are placed and rendered. In XR, this is the main scene; on desktop, it may be rendered within a window.

**Responsibilities**:

- Manage a collection of 3D `Entity3D` objects.
- Provide methods to add, retrieve, and remove `Entity3D` objects.
- Handle rendering of all `Entity3D` objects in the scene.

**Code**:

```cpp
/**
 * @class Scene3D
 * @brief Represents a 3D environment where 3D entities are placed and rendered.
 * 
 * In XR, this is the main scene; on desktop, it may be rendered within a window.
 * The Scene3D class manages a collection of 3D entities and handles their rendering.
 */
class Scene3D {
private:
    std::vector<std::unique_ptr<Entity3D>> _entities;   ///< Collection of 3D entities in the scene
public:
    /**
     * @brief Adds a new 3D entity to the scene.
     * @param entity Unique pointer to the Entity3D to add.
     */
    void addEntity(std::unique_ptr<Entity3D> entity);
    
    /**
     * @brief Retrieves an entity by name.
     * @param name The name of the entity to retrieve.
     * @return Optional reference to the Entity3D if found.
     */
    std::optional<std::reference_wrapper<Entity3D>> getEntity(std::string_view name) const;
    
    /**
     * @brief Removes an entity by name.
     * @param name The name of the entity to remove.
     */
    void removeEntity(std::string_view name);
    
    /**
     * @brief Gets all entities in the scene.
     * @return Const reference to the vector of all entities.
     */
    const std::vector<std::unique_ptr<Entity3D>>& getEntities() const { return _entities; }

    /**
     * @brief Renders all entities in the scene.
     * @note Platform-specific implementation required.
     */
    virtual void render();
};
```

---

## **3. UIPanel**

**Goal**:
The `UIPanel` class represents a 2D surface (e.g., a traditional OS window or a 2D panel in XR). It contains `UIWidget`s (2D elements) and handles their rendering and interaction.

**Responsibilities**:

- Manage a collection of 2D `UIWidget`s.
- Provide methods to add, retrieve, and remove `UIWidget`s.
- Handle rendering of all `UIWidget`s in the panel.

**Code**:

```cpp
/**
 * @class UIPanel
 * @brief Represents a 2D surface (e.g., a traditional OS window or a 2D panel in XR).
 * 
 * The UIPanel class contains UI widgets (2D elements) and handles their rendering and interaction.
 */
class UIPanel {
private:
    std::vector<std::unique_ptr<UIWidget>> _widgets;    ///< Collection of 2D UI widgets in the panel
public:
    /**
     * @brief Adds a new UI widget to the panel.
     * @param widget Unique pointer to the UIWidget to add.
     */
    void addWidget(std::unique_ptr<UIWidget> widget);

    /**
     * @brief Renders all widgets in the panel.
     * @note Platform-specific implementation required.
     */
    virtual void render();
};
```

---

## **4. UIWidget**

**Goal**:
The `UIWidget` class is an abstract base for all 2D UI elements (e.g., buttons, text labels, images) that can be rendered in a `UIPanel`. It defines common properties like position and size, and declares virtual methods for rendering and interaction.

**Responsibilities**:

- Store 2D position and size.
- Define a contract for rendering and handling user interactions (e.g., clicks).

**Code**:

```cpp
/**
 * @class UIWidget
 * @brief Abstract base class for all 2D UI elements (e.g., buttons, text labels, images).
 * 
 * The UIWidget class defines common properties like position and size, and declares
 * virtual methods for rendering and interaction that must be implemented by derived classes.
 */
class UIWidget {
private:
    Vector<2, float> _position;     ///< 2D position of the widget
    Vector<2, float> _size;         ///< 2D size of the widget
public:
    /**
     * @brief Gets the position of the widget.
     * @return Const reference to the 2D position vector.
     */
    const Vector<2, float>& getPosition() const { return _position; }
    
    /**
     * @brief Sets the position of the widget.
     * @param position The new 2D position vector.
     */
    void setPosition(const Vector<2, float>& position) { _position = position; }
    
    /**
     * @brief Gets the size of the widget.
     * @return Const reference to the 2D size vector.
     */
    const Vector<2, float>& getSize() const { return _size; }
    
    /**
     * @brief Sets the size of the widget.
     * @param size The new 2D size vector.
     */
    void setSize(const Vector<2, float>& size) { _size = size; }

    /**
     * @brief Renders the widget.
     * @note Must be implemented by derived classes with platform-specific rendering logic.
     */
    virtual void render() = 0;
    
    /**
     * @brief Handles click interaction on the widget.
     * @note Must be implemented by derived classes to define click behavior.
     */
    virtual void onClick() = 0;
};
```

---

## **5. Entity3D**

**Goal**:
The `Entity3D` class is an abstract base for all 3D entities (e.g., 3D models, meshes, spatial UI elements) that can be placed in a `Scene3D`. It defines common properties like 3D position and size, and declares virtual methods for rendering and interaction.

**Responsibilities**:

- Store 3D position and size.
- Define a contract for rendering and handling user interactions (e.g., gestures, gazes).

**Code**:

```cpp
/**
 * @class Entity3D
 * @brief Abstract base class for all 3D entities (e.g., 3D models, meshes, spatial UI elements).
 * 
 * The Entity3D class defines common properties like 3D position and size, and declares
 * virtual methods for rendering and interaction that must be implemented by derived classes.
 */
class Entity3D {
private:
    Vector<3, float> _position;     ///< 3D position of the entity
    Vector<3, float> _size;         ///< 3D size of the entity
public:
    /**
     * @brief Gets the position of the entity.
     * @return Const reference to the 3D position vector.
     */
    const Vector<3, float>& getPosition() const { return _position; }
    
    /**
     * @brief Sets the position of the entity.
     * @param position The new 3D position vector.
     */
    void setPosition(const Vector<3, float>& position) { _position = position; }
    
    /**
     * @brief Gets the size of the entity.
     * @return Const reference to the 3D size vector.
     */
    const Vector<3, float>& getSize() const { return _size; }
    
    /**
     * @brief Sets the size of the entity.
     * @param size The new 3D size vector.
     */
    void setSize(const Vector<3, float>& size) { _size = size; }

    /**
     * @brief Renders the entity.
     * @note Must be implemented by derived classes with platform-specific rendering logic.
     */
    virtual void render() = 0;
    
    /**
     * @brief Handles interaction with the entity (e.g., gestures, gazes).
     * @note Must be implemented by derived classes to define interaction behavior.
     */
    virtual void onInteract() = 0;
};
```

---

## **Summary Table**

| Class              | Role                                                                 | Key Features                                                                 |
|--------------------|----------------------------------------------------------------------|-------------------------------------------------------------------------------|
| Application | Root of the app; manages `Scene3D` and `UIPanel`s.                   | Add/remove panels, access scene, high-level app lifecycle.                   |
| Scene3D            | 3D environment for `Entity3D` objects.                               | Add/remove entities, render all entities.                                    |
| UIPanel            | 2D surface for `UIWidget`s.                                          | Add/remove widgets, render all widgets.                                      |
| UIWidget           | Abstract base for 2D UI elements.                                    | Position/size, render, handle clicks.                                        |
| Entity3D           | Abstract base for 3D entities.                                       | 3D position/size, render, handle interactions.                               |

---

## **Example Usage**

```cpp
/**
 * @class MyApp
 * @brief Custom application implementation.
 */
class MyApp : public Application {
public:
    /**
     * @brief Runs the application.
     * 
     * Creates a main UI panel, adds a button widget to it, and renders the panel.
     */
    void run() {
        addPanel("MainPanel");
        auto panel = getPanel("MainPanel");
        if (panel) {
            panel->get().addWidget(std::make_unique<ButtonWidget>());
            panel->get().render();
        }
    }
};

/**
 * @class ButtonWidget
 * @brief Custom 2D button widget implementation.
 */
class ButtonWidget : public UIWidget {
public:
    /**
     * @brief Renders the button widget.
     * @note Implement platform-specific 2D rendering logic.
     */
    void render() override { /* Platform-specific 2D rendering */ }
    
    /**
     * @brief Handles button click events.
     */
    void onClick() override { /* Handle button click */ }
};

/**
 * @class MeshEntity
 * @brief Custom 3D mesh entity implementation.
 */
class MeshEntity : public Entity3D {
public:
    /**
     * @brief Renders the 3D mesh entity.
     * @note Implement platform-specific 3D rendering logic.
     */
    void render() override { /* Platform-specific 3D rendering */ }
    
    /**
     * @brief Handles interaction with the 3D mesh entity.
     */
    void onInteract() override { /* Handle 3D interaction */ }
};
```
