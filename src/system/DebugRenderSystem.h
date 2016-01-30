#pragma once

#include "ecs/Include.h"
#include "component/Include.h"
#include "math/Geometry.h"
#include "system/Events.h"
#include "opengl/BufferObject.h"

#include <vector>

namespace pg {

struct Context;

namespace system {

class DebugRenderSystem : public ecs::System<DebugRenderSystem>, public ecs::Receiver {
public:
    DebugRenderSystem() = delete;
    explicit DebugRenderSystem(Context& context);
    void configure(ecs::EventManager&) override;
    void update(ecs::EntityManager&, ecs::EventManager&, float) override;
    void receive(const ecs::ComponentAssignedEvent<component::Camera>&);
    void receive(const ShowDebugLines&);
    void receive(const ShowDebugBoxes&);
    void receive(const RenderDebugLine&);

private:
    Context&                    context_;
    ecs::Entity                 cameraEntity_;
    math::Matrix4f              defaultProjection_;
    std::vector<math::Line>     debugLines_;
    std::vector<float>          lineLifeTimes_;
    opengl::BufferObject        lineBuffer_;
    opengl::VertexArrayObject   lineBufferArray_;

    bool    showLines_;
    bool    showBoxes_;
};

}
}