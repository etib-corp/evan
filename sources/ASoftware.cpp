/*
** ETIB PROJECT, 2025
** maverik
** File description:
** ASoftware
*/

#include "ASoftware.hpp"

std::shared_ptr<maverik::AGraphicalContext>
maverik::ASoftware::getGraphicalContext() {
  return this->_graphicalContext;
}
