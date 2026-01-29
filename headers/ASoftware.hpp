/*
** ETIB PROJECT, 2025
** evan
** File description:
** ASoftware
*/

#pragma once

#include "AGraphicalContext.hpp"
#include "EvanPlatform.hpp"
#include <iostream>

#include "Version.hpp"

namespace evan {
/**
 * @brief Abstract base class representing a software application within the
 * evan namespace.
 *
 * This class provides a common interface for software applications, including
 * access to graphical context and versioning information for both the
 * application and its engine.
 */
class ASoftware {
public:
  virtual ~ASoftware() = default;

  /**
   * @brief Retrieves the graphical context associated with the software
   * application.
   *
   * This function returns a shared pointer to an AGraphicalContext instance,
   * which represents the graphical context used by the application.
   *
   * @return A shared pointer to the AGraphicalContext instance.
   */
  std::shared_ptr<AGraphicalContext> getGraphicalContext();

  std::vector<Vertex> render(std::vector<Primitive> objects);

  virtual void drawFrame(std::vector<Vertex> vertices) = 0;

  void run();

  virtual void pollEvents() = 0;

  virtual bool shouldClose() = 0;

protected:
  std::string _appName;    // Application name (defined by user)
  Version *_appVersion;    // Application version (major, minor, patch)
  std::string _engineName; // Engine name (defined by us WIP)
  Version *_engineVersion; // Engine version   (major, minor, patch)

  std::shared_ptr<AGraphicalContext> _graphicalContext; // Graphical context
};
} // namespace evan
