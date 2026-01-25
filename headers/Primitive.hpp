/*
** ETIB PROJECT, 2026
** evan
** File description:
** Primitive
*/

#pragma once

#include "Vertex.hpp"

namespace evan {
    class Primitive {
    public:
        virtual ~Primitive() = default;

        virtual void move(const glm::vec3 &delta, float frameRate)
        {
            _position += delta * frameRate;
        }

        virtual void setPosition(const glm::vec3 &position)
        {
            _position = position;
        }

        virtual void setRotation(const glm::vec3 &rotation)
        {
            _rotation = rotation;
        }

        virtual void rotate(const glm::vec3 &delta)
        {
            _rotation += delta;
        }

        virtual void rescale(const glm::vec3 &factor)
        {
            _scale *= factor;
        }

        virtual void setScale(const glm::vec3 &scale)
        {
            _scale = scale;
        }

        virtual void hide()
        {
            _isHidden = true;
        }

        virtual void show()
        {
            _isHidden = false;
        }

        virtual glm::vec3 getColor() const
        {
            return _color;
        }

        virtual void setColor(const glm::vec3 &color)
        {
            _color = color;
        }

        virtual glm::mat4 getMatrix() const
        {
            glm::mat4 model = glm::mat4(1.0f);
            model = glm::translate(model, _position);
            model = glm::rotate(model, glm::radians(_rotation.x), glm::vec3(1.0f, 0.0f, 0.0f));
            model = glm::rotate(model, glm::radians(_rotation.y), glm::vec3(0.0f, 1.0f, 0.0f));
            model = glm::rotate(model, glm::radians(_rotation.z), glm::vec3(0.0f, 0.0f, 1.0f));
            model = glm::scale(model, _scale);
            return model;
        }

    protected:
        glm::vec3 _position;
        glm::vec3 _rotation;
        glm::vec3 _scale;
        glm::vec3 _color;
        std::vector<evan::Vertex> _vertices;
        bool _isHidden = false;
    };
} // namespace evan
