#include "imgui/imgui.h"
#include "Wrenly.h"
#include "app/MouseEvents.h"
#include "component/Include.h"
#include "math/Vector.h"
#include "manager/MeshManager.h"
#include "manager/ShaderManager.h"
#include "opengl/VertexArrayObjectFactory.h"
#include "system/ScriptSystem.h"
#include "system/PickingSystem.h"
#include "system/WrenBindingsImpl.h"
#include "ecs/Include.h"
#include "utils/Locator.h"
#include "utils/Assert.h"
#include "utils/Log.h"
#include "utils/RingBuffer.h"
extern "C" {
#include <wren.h>
}
#include <vector>
#include <cstdlib>

namespace pg {
namespace wren {

using component::Transform;
using component::Renderable;

/***
 *       _  __           __           ___
 *      / |/ /_ ____ _  / /  ___ ____/ _ | ___________ ___ __
 *     /    / // /  ' \/ _ \/ -_) __/ __ |/ __/ __/ _ `/ // /
 *    /_/|_/\_,_/_/_/_/_.__/\__/_/ /_/ |_/_/ /_/  \_,_/\_, /
 *                                                    /___/
 */

void arrayPushBack(WrenVM* vm) {
    std::vector<float>* array = wrenly::getForeignSlotPtr<std::vector<float>, 0>(vm);
    array->push_back(float(wrenGetSlotDouble(vm, 1)));
}

void arrayAt(WrenVM* vm) {
    const std::vector<float>* array = wrenly::getForeignSlotPtr<std::vector<float>, 0>(vm);
    std::size_t i = (std::size_t)wrenGetSlotDouble(vm, 1);
    if (i >= array->size()) {
        LOG_ERROR << "Script error> NumberArray: index out of bounds. Size: " << array->size() << ", index: " << i;
        return;
    }
    wrenSetSlotDouble(vm, 0, double(array->at(i)));
}

/***
*       ____     __  _ __
*      / __/__  / /_(_) /___ __
*     / _// _ \/ __/ / __/ // /
*    /___/_//_/\__/_/\__/\_, /
*                       /___/
*/

void set(WrenVM* vm) {
    ecs::Entity* e = wrenly::getForeignSlotPtr<ecs::Entity, 0>(vm);
    uint32_t id = static_cast<uint32_t>(wrenGetSlotDouble(vm, 1));
    ecs::EntityManager* entityManager = Locator< ecs::EntityManager >::get();
    *e = entityManager->get(id);
}

void entityIndex(WrenVM* vm) {
    const ecs::Entity* e = wrenly::getForeignSlotPtr<ecs::Entity, 0>(vm);
    wrenSetSlotDouble(vm, 0, double(e->id().index()));
}

void entityVersion(WrenVM* vm) {
    const ecs::Entity* e = wrenly::getForeignSlotPtr<ecs::Entity, 0>(vm);
    wrenSetSlotDouble(vm, 0, double(e->id().version()));
}

void setTransform(WrenVM* vm) {
    ecs::Entity* e = wrenly::getForeignSlotPtr<ecs::Entity, 0>(vm);
    component::Transform* t = wrenly::getForeignSlotPtr<Transform, 1>(vm);
    *e->componentPointer< component::Transform >() = *t;
}

void hasTransform(WrenVM* vm) {
    const ecs::Entity* e = wrenly::getForeignSlotPtr<ecs::Entity, 0>(vm);
    wrenSetSlotBool(vm, 0, e->has<component::Transform>());
}

void hasRenderable(WrenVM* vm) {
    const ecs::Entity* e = wrenly::getForeignSlotPtr<ecs::Entity, 0>(vm);
    wrenSetSlotBool(vm, 0, e->has<component::Renderable>());
}

void hasCamera(WrenVM* vm) {
    const ecs::Entity* e = wrenly::getForeignSlotPtr<ecs::Entity, 0>(vm);
    wrenSetSlotBool(vm, 0, e->has<component::Camera>());
}

void hasPointLight(WrenVM* vm) {
    const ecs::Entity* e = wrenly::getForeignSlotPtr<ecs::Entity, 0>(vm);
    wrenSetSlotBool(vm, 0, e->has<component::PointLight>());
}

void assignTransform(WrenVM* vm) {
    ecs::Entity* e = wrenly::getForeignSlotPtr<ecs::Entity, 0>(vm);
    const component::Transform* t = wrenly::getForeignSlotPtr<Transform, 1>(vm);
    e->assign<component::Transform>(t->position, t->rotation, t->scale);
}

void assignRenderable(WrenVM* vm) {
    ecs::Entity* e = wrenly::getForeignSlotPtr<ecs::Entity, 0>(vm);
    const WrenRenderable* r = wrenly::getForeignSlotPtr<WrenRenderable, 1>(vm);

    pg::opengl::BufferObject* buffer = Locator<pg::MeshManager>::get()->get(r->model.cString());
    pg::opengl::Program* shader = Locator<pg::ShaderManager>::get()->get(r->shader.cString());

    std::unordered_map<std::string, float> uniforms{};
    uniforms.emplace("specColor_r", r->specularColor.r);
    uniforms.emplace("specColor_g", r->specularColor.g);
    uniforms.emplace("specColor_b", r->specularColor.b);
    uniforms.emplace("ambientColor_r", r->ambientColor.r);
    uniforms.emplace("ambientColor_g", r->ambientColor.g);
    uniforms.emplace("ambientColor_b", r->ambientColor.b);
    uniforms.emplace("baseColor_r", r->baseColor.r);
    uniforms.emplace("baseColor_g", r->baseColor.g);
    uniforms.emplace("baseColor_b", r->baseColor.b);
    system::Material mat;
    mat.type = system::MaterialType::Specular;
    mat.uniforms = uniforms;
    opengl::VertexArrayObjectFactory factory{ buffer, shader };
    factory.addStandardAttribute(opengl::VertexAttribute::Vertex);
    factory.addStandardAttribute(opengl::VertexAttribute::Normal);
    auto vao = factory.getVao();

    e->assign<component::Renderable>(buffer, shader, vao, mat);

    const auto& bb = Locator<MeshManager>::get()->getBoundingBox(r->model.cString());
    e->assign<math::AABoxf>(bb.min, bb.max);
}

void getTransform(WrenVM* vm) {
    const ecs::Entity* entity = wrenly::getForeignSlotPtr<ecs::Entity, 0>(vm);
    if (entity->has<Transform>()) {
        wrenly::setForeignSlotValue(vm, *entity->component<Transform>());
    }
    else {
        LOG_INFO << "Script error: entity " << entity->id().index() << " doesn't have a transform component";
    }
}

/***
*       ____     __  _ __       __  ___
*      / __/__  / /_(_) /___ __/  |/  /__ ____  ___ ____ ____ ____
*     / _// _ \/ __/ / __/ // / /|_/ / _ `/ _ \/ _ `/ _ `/ -_) __/
*    /___/_//_/\__/_/\__/\_, /_/  /_/\_,_/_//_/\_,_/\_, /\__/_/
*                       /___/                      /___/
*/

void createEntity(WrenVM* vm) {
    ecs::Entity entity = Locator<ecs::EntityManager>::get()->create();
    wrenGetVariable(vm, "builtin/entity", "Entity", 0);
    void* data = wrenSetSlotNewForeign(vm, 0, 0, sizeof(ecs::Entity));
    memcpy(data, (void*)&entity, sizeof(ecs::Entity));
}

void entityCount(WrenVM* vm) {
    wrenSetSlotDouble(vm, 0, double(Locator<ecs::EntityManager>::get()->size()));
}

/***
*       ____              __  __  ___
*      / __/  _____ ___  / /_/  |/  /__ ____  ___ ____ ____ ____
*     / _/| |/ / -_) _ \/ __/ /|_/ / _ `/ _ \/ _ `/ _ `/ -_) __/
*    /___/|___/\__/_//_/\__/_/  /_/\_,_/_//_/\_,_/\_, /\__/_/
*                                                /___/
*/

void listenToKeyDown(WrenVM* vm) {
    ecs::Entity* entity = wrenly::getForeignSlotPtr<ecs::Entity, 1>(vm);
    Locator<system::ScriptSystem>::get()->listenToKeyDown(wrenGetSlotString(vm, 2), entity);
}

void listenToKeyPressed(WrenVM* vm) {
    ecs::Entity* entity = wrenly::getForeignSlotPtr<ecs::Entity, 1>(vm);
    Locator<system::ScriptSystem>::get()->listenToKeyPressed(wrenGetSlotString(vm, 2), entity);
}

void listenToKeyUp(WrenVM* vm) {
    ecs::Entity* entity = wrenly::getForeignSlotPtr<ecs::Entity, 1>(vm);
    Locator<system::ScriptSystem>::get()->listenToKeyUp(wrenGetSlotString(vm, 2), entity);
}

void listenToMouseDown(WrenVM* vm) {
    ecs::Entity* entity = wrenly::getForeignSlotPtr<ecs::Entity, 1>(vm);
    Locator<system::ScriptSystem>::get()->listenToMouseDown(wrenGetSlotString(vm, 2), entity);
}

void listenToMousePressed(WrenVM* vm) {
    ecs::Entity* entity = wrenly::getForeignSlotPtr<ecs::Entity, 1>(vm);
    Locator<system::ScriptSystem>::get()->listenToMousePressed(wrenGetSlotString(vm, 2), entity);
}

void listenToMouseUp(WrenVM* vm) {
    ecs::Entity* entity = wrenly::getForeignSlotPtr<ecs::Entity, 1>(vm);
    Locator<system::ScriptSystem>::get()->listenToMouseUp(wrenGetSlotString(vm, 2), entity);
}

void mouseX(WrenVM* vm) {
    math::Vec2f coords = Locator<MouseEvents>::get()->getNormalizedMouseCoords();
    wrenSetSlotDouble(vm, 0, double(coords.x));
}

void mouseY(WrenVM* vm) {
    math::Vec2f coords = Locator<MouseEvents>::get()->getNormalizedMouseCoords();
    wrenSetSlotDouble(vm, 0, double(coords.y));
}

void mouseDx(WrenVM* vm) {
    math::Vec2f coords = Locator<MouseEvents>::get()->getMouseDelta();
    wrenSetSlotDouble(vm, 0, double(coords.x));
}

void mouseDy(WrenVM* vm) {
    math::Vec2f coords = Locator<MouseEvents>::get()->getMouseDelta();
    wrenSetSlotDouble(vm, 0, double(coords.y));
}

/***
 *       ____                _
 *      /  _/_ _  ___ ___ __(_)
 *     _/ //  ' \/ _ `/ // / /
 *    /___/_/_/_/\_, /\_,_/_/
 *              /___/
 */

void begin(WrenVM* vm) {
    ImGui::Begin((const char*)wrenGetSlotString(vm, 1), NULL, 0);
}

void beginWithFlags(WrenVM* vm) {
    ImGui::Begin(
        (const char*)wrenGetSlotString(vm, 1),
        NULL,
        *(std::uint32_t*)wrenly::getForeignSlotPtr<ImGuiWindowFlag, 2>(vm)
        );
}

void setWindowPos(WrenVM* vm) {
    ImGui::SetNextWindowPos(*(const ImVec2*)wrenly::getForeignSlotPtr<math::Vec2i, 1>(vm));
}

void setWindowSize(WrenVM* vm) {
    ImGui::SetNextWindowSize(*(const ImVec2*)wrenly::getForeignSlotPtr<math::Vec2i, 1>(vm));
}

void text(WrenVM* vm) {
    const char* str = wrenGetSlotString(vm, 1);
    ImGui::Text(str);
}

void dummy(WrenVM* vm) {
    const math::Vec2f* v = wrenly::getForeignSlotPtr<math::Vec2f, 1>(vm);
    ImGui::Dummy((const ImVec2&)(*v));
}

void textColored(WrenVM* vm) {
    // do nothing for now
    //const math::Vector2f* v = (const math::Vector2f*)wrenGetSlotForeign( vm, 1 );
    //const char* t = wrenGetSlotString( vm, 2 );
    //ImGui::TextColored();
}

void bulletText(WrenVM* vm) {
    ImGui::BulletText((const char*)wrenGetSlotString(vm, 1));
}

void button(WrenVM* vm) {
    bool pressed = ImGui::Button((const char*)wrenGetSlotString(vm, 1));
    wrenSetSlotBool(vm, 0, pressed);
}

void buttonSized(WrenVM* vm) {
    ImGui::Button(
        (const char*)wrenGetSlotString(vm, 1),
        *(const ImVec2*)wrenly::getForeignSlotPtr<math::Vec2i, 2>(vm)
        );
}

void plotNumberArray(WrenVM* vm) {
    const std::vector<float>* array = wrenly::getForeignSlotPtr<std::vector<float>, 2>(vm);
    ImGui::PlotLines(
        wrenGetSlotString(vm, 1),
        array->data(),
        int(wrenGetSlotDouble(vm, 3)),
        0, NULL, FLT_MAX, FLT_MAX, ImVec2(140, 80), sizeof(float)
        );
}

void plotNumberArrayWithOffset(WrenVM* vm) {
    const std::vector<float>* array = wrenly::getForeignSlotPtr<std::vector<float>, 2>(vm);
    ImGui::PlotLines(
        wrenGetSlotString(vm, 1), array->data(),
        int(wrenGetSlotDouble(vm, 3)), int(wrenGetSlotDouble(vm, 4)),
        NULL, FLT_MAX, FLT_MAX, ImVec2(140, 80), sizeof(float)
        );
}

void plotNumberArrayWithOffsetAndSize(WrenVM* vm) {
    const std::vector<float>* array = wrenly::getForeignSlotPtr<std::vector<float>, 2>(vm);
    ImGui::PlotLines(
        wrenGetSlotString(vm, 1), array->data(),
        int(wrenGetSlotDouble(vm, 3)), int(wrenGetSlotDouble(vm, 4)),
        NULL, FLT_MAX, FLT_MAX,
        *(const ImVec2*)wrenly::getForeignSlotPtr<math::Vec2i, 5>(vm),
        sizeof(float)
        );
}

void plotRingBuffer(WrenVM* vm) {
    RingBuffer<float>* buffer = wrenly::getForeignSlotPtr<RingBuffer<float>, 2>(vm);
    BasicStorage<float> numbers{ buffer->size() };
    for (unsigned int i = 0; i < buffer->size(); i++) {
        *numbers.at(i) = buffer->at(i);
    }
    ImGui::PlotLines(
        wrenGetSlotString(vm, 1), numbers.at(0),
        buffer->size(), 0,
        NULL, FLT_MAX, FLT_MAX,
        ImVec2(140, 80),
        sizeof(float)
        );
}

void plotRingBufferWithSize(WrenVM* vm) {
    RingBuffer<float>* buffer = wrenly::getForeignSlotPtr<RingBuffer<float>, 2>(vm);
    BasicStorage<float> numbers{ buffer->size() };
    for (unsigned int i = 0; i < buffer->size(); i++) {
        *numbers.at(i) = buffer->at(i);
    }
    ImGui::PlotLines(
        wrenGetSlotString(vm, 1), numbers.at(0),
        buffer->size(), 0,
        NULL, FLT_MAX, FLT_MAX,
        *(const ImVec2*)wrenly::getForeignSlotPtr<math::Vec2i, 3>(vm),
        sizeof(float)
        );
}

void sliderFloat(WrenVM* vm) {
    const char* label = wrenGetSlotString(vm, 1);
    float value = float(wrenGetSlotDouble(vm, 2));
    float min =   float(wrenGetSlotDouble(vm, 3));
    float max =   float(wrenGetSlotDouble(vm, 4));
    ImGui::SliderFloat(label, &value, min, max);
    wrenSetSlotDouble(vm, 0, value);
}

void setTitleBar(WrenVM* vm) {
    std::uint32_t* bits = reinterpret_cast<std::uint32_t*>(wrenly::getForeignSlotPtr<ImGuiWindowFlag, 0>(vm));
    *bits &= ~ImGuiWindowFlags_NoTitleBar;
}

void unsetTitleBar(WrenVM* vm) {
    std::uint32_t* bits = reinterpret_cast<std::uint32_t*>(wrenly::getForeignSlotPtr<ImGuiWindowFlag, 0>(vm));
    (*bits) |= ImGuiWindowFlags_NoTitleBar;
}

void setResize(WrenVM* vm) {
    std::uint32_t* bits = reinterpret_cast<std::uint32_t*>(wrenly::getForeignSlotPtr<ImGuiWindowFlag, 0>(vm));
    *bits &= ~ImGuiWindowFlags_NoResize;
}

void setMove(WrenVM* vm) {
    std::uint32_t* bits = reinterpret_cast<std::uint32_t*>(wrenly::getForeignSlotPtr<ImGuiWindowFlag, 0>(vm));
    *bits &= ~ImGuiWindowFlags_NoMove;
}

void unsetMove(WrenVM* vm) {
    std::uint32_t* bits = reinterpret_cast<std::uint32_t*>(wrenly::getForeignSlotPtr<ImGuiWindowFlag, 0>(vm));
    *bits |= ImGuiWindowFlags_NoMove;
}

void unsetResize(WrenVM* vm) {
    std::uint32_t* bits = reinterpret_cast<std::uint32_t*>(wrenly::getForeignSlotPtr<ImGuiWindowFlag, 0>(vm));
    *bits |= ImGuiWindowFlags_NoResize;
}

void setShowBorders(WrenVM* vm) {
    std::uint32_t* bits = reinterpret_cast<std::uint32_t*>(wrenly::getForeignSlotPtr<ImGuiWindowFlag, 0>(vm));
    *bits |= ImGuiWindowFlags_ShowBorders;
}

void unsetShowBorders(WrenVM* vm) {
    std::uint32_t* bits = reinterpret_cast<std::uint32_t*>(wrenly::getForeignSlotPtr<ImGuiWindowFlag, 0>(vm));
    *bits &= ~ImGuiWindowFlags_ShowBorders;
}

/***
 *      ____            __               _
 *     / __ \__ _____ _/ /____ _______  (_)__  ___
 *    / /_/ / // / _ `/ __/ -_) __/ _ \/ / _ \/ _ \
 *    \___\_\_,_/\_,_/\__/\__/_/ /_//_/_/\___/_//_/
 *
 */

void conjugate(WrenVM* vm) {
    wrenEnsureSlots(vm, 1);
    const math::Quatf* q = wrenly::getForeignSlotPtr<math::Quatf, 0>(vm);
    math::Quatf res = q->conjugate();
    wrenly::setForeignSlotValue(vm, res);
}

void inverse(WrenVM* vm) {
    wrenEnsureSlots(vm, 1);
    const math::Quatf* q = wrenly::getForeignSlotPtr<math::Quatf, 0>(vm);
    math::Quatf res = q->inverse();
    wrenly::setForeignSlotValue(vm, res);
}

void multiply(WrenVM* vm) {
    wrenEnsureSlots(vm, 2);
    const math::Quatf* lhs = wrenly::getForeignSlotPtr<math::Quatf, 0>(vm);
    const math::Quatf* rhs = wrenly::getForeignSlotPtr<math::Quatf, 1>(vm);
    math::Quatf res = lhs->multiply(*rhs);
    wrenly::setForeignSlotValue(vm, res);
}

void axis(WrenVM* vm) {
    wrenEnsureSlots(vm, 1);
    const math::Quatf* q = wrenly::getForeignSlotPtr<math::Quatf, 0>(vm);
    math::Vec3f res = q->axis();
    wrenly::setForeignSlotValue(vm, res);
}

void getQuatReal(WrenVM* vm) {
    const math::Quatf* q = wrenly::getForeignSlotPtr<math::Quatf, 0>(vm);
    wrenly::setForeignSlotValue(vm, q->v);
}

void xaxis(WrenVM* vm) {
    const math::Quatf* q = wrenly::getForeignSlotPtr<math::Quatf, 0>(vm);
    wrenly::setForeignSlotValue(vm, q->xaxis());
}

void yaxis(WrenVM* vm) {
    const math::Quatf* q = wrenly::getForeignSlotPtr<math::Quatf, 0>(vm);
    wrenly::setForeignSlotValue(vm, q->yaxis());
}

void zaxis(WrenVM* vm) {
    const math::Quatf* q = wrenly::getForeignSlotPtr<math::Quatf, 0>(vm);
    wrenly::setForeignSlotValue(vm, q->zaxis());
}

/***
 *       ___  _           __        ______
 *      / _ \(_)__  ___ _/ /  __ __/ _/ _/__ ____
 *     / , _/ / _ \/ _ `/ _ \/ // / _/ _/ -_) __/
 *    /_/|_/_/_//_/\_, /_.__/\_,_/_//_/ \__/_/
 *                /___/
 */

void ringBufferPushBack(WrenVM* vm) {
    RingBuffer<float>* ring = wrenly::getForeignSlotPtr<RingBuffer<float>, 0>(vm);
    ring->pushBack(float(wrenGetSlotDouble(vm, 1)));
}

/***
 *      _   __        __
 *     | | / /__ ____/ /____  ____
 *     | |/ / -_) __/ __/ _ \/ __/
 *     |___/\__/\__/\__/\___/_/
 *
 */

void hadamard2f(WrenVM* vm) {
    wrenEnsureSlots(vm, 2);
    const math::Vec2f* lhs = wrenly::getForeignSlotPtr<math::Vec2f, 0>(vm);
    const math::Vec2f* rhs = wrenly::getForeignSlotPtr<math::Vec2f, 1>(vm);
    math::Vec2f res = lhs->hadamard(*rhs);
    wrenly::setForeignSlotValue(vm, res);
}

void plus2f(WrenVM* vm) {
    wrenEnsureSlots(vm, 2);
    const math::Vec2f* lhs = wrenly::getForeignSlotPtr<math::Vec2f, 0>(vm);
    const math::Vec2f* rhs = wrenly::getForeignSlotPtr<math::Vec2f, 1>(vm);
    math::Vec2f res = lhs->operator+(*rhs);
    wrenly::setForeignSlotValue(vm, res);
}

void minus2f(WrenVM* vm) {
    wrenEnsureSlots(vm, 2);
    const math::Vec2f* lhs = wrenly::getForeignSlotPtr<math::Vec2f, 0>(vm);
    const math::Vec2f* rhs = wrenly::getForeignSlotPtr<math::Vec2f, 1>(vm);
    math::Vec2f res = lhs->operator-(*rhs);
    wrenly::setForeignSlotValue(vm, res);
}

void cross3f(WrenVM* vm) {
    wrenEnsureSlots(vm, 2);
    const math::Vec3f* lhs = wrenly::getForeignSlotPtr<math::Vec3f, 0>(vm);
    const math::Vec3f* rhs = wrenly::getForeignSlotPtr<math::Vec3f, 1>(vm);
    math::Vec3f res = lhs->cross(*rhs);
    wrenly::setForeignSlotValue(vm, res);
}

void plus3f(WrenVM* vm) {
    wrenEnsureSlots(vm, 2);
    const math::Vec3f* lhs = wrenly::getForeignSlotPtr<math::Vec3f, 0>(vm);
    const math::Vec3f* rhs = wrenly::getForeignSlotPtr<math::Vec3f, 1>(vm);
    math::Vec3f res = lhs->operator+(*rhs);
    wrenly::setForeignSlotValue(vm, res);
}

void minus3f(WrenVM* vm) {
    wrenEnsureSlots(vm, 2);
    const math::Vec3f* lhs = wrenly::getForeignSlotPtr<math::Vec3f, 0>(vm);
    const math::Vec3f* rhs = wrenly::getForeignSlotPtr<math::Vec3f, 1>(vm);
    math::Vec3f res = lhs->operator-(*rhs);
    wrenly::setForeignSlotValue(vm, res);
}

void hadamard3f(WrenVM* vm) {
    wrenEnsureSlots(vm, 2);
    const math::Vec3f* lhs = wrenly::getForeignSlotPtr<math::Vec3f, 0>(vm);
    const math::Vec3f* rhs = wrenly::getForeignSlotPtr<math::Vec3f, 1>(vm);
    math::Vec3f res = lhs->hadamard(*rhs);
    wrenly::setForeignSlotValue(vm, res);
}

void hadamard4f(WrenVM* vm) {
    wrenEnsureSlots(vm, 2);
    const math::Vec4f* lhs = wrenly::getForeignSlotPtr<math::Vec4f, 0>(vm);
    const math::Vec4f* rhs = wrenly::getForeignSlotPtr<math::Vec4f, 1>(vm);
    math::Vec4f res = lhs->hadamard(*rhs);
    wrenly::setForeignSlotValue(vm, res);
}

void plus4f(WrenVM* vm) {
    wrenEnsureSlots(vm, 2);
    const math::Vec4f* lhs = wrenly::getForeignSlotPtr<math::Vec4f, 0>(vm);
    const math::Vec4f* rhs = wrenly::getForeignSlotPtr<math::Vec4f, 1>(vm);
    math::Vec4f res = lhs->operator+(*rhs);
    wrenly::setForeignSlotValue(vm, res);
}

void minus4f(WrenVM* vm) {
    wrenEnsureSlots(vm, 2);
    const math::Vec4f* lhs = wrenly::getForeignSlotPtr<math::Vec4f, 0>(vm);
    const math::Vec4f* rhs = wrenly::getForeignSlotPtr<math::Vec4f, 1>(vm);
    math::Vec4f res = lhs->operator-(*rhs);
    wrenly::setForeignSlotValue(vm, res);
}

/***
*       ____         __
*      / __/_ _____ / /____ __ _  ___
*     _\ \/ // (_-</ __/ -_)  ' \(_-<
*    /___/\_, /___/\__/\__/_/_/_/___/
*        /___/
*/

// Pick3d
void castCameraRay(WrenVM* vm) {
    ecs::EntityManager* entities = Locator<ecs::EntityManager>::get();
    ecs::EventManager* events = Locator<ecs::EventManager>::get();
    system::PickingSystem* pick3d = Locator<system::PickingSystem>::get();
    float x = float(wrenGetSlotDouble(vm, 1));
    float y = float(wrenGetSlotDouble(vm, 2));
    ecs::Entity res = pick3d->rayCast(*entities, *events, x, y);
    wrenGetVariable(vm, "builtin/entity", "Entity", 0);
    wrenly::setForeignSlotValue(vm, res);
}

/***
*       ______                                             __
*      / ____/___  ____ ___  ____  ____  ____  ___  ____  / /______
*     / /   / __ \/ __ `__ \/ __ \/ __ \/ __ \/ _ \/ __ \/ __/ ___/
*    / /___/ /_/ / / / / / / /_/ / /_/ / / / /  __/ / / / /_(__  )
*    \____/\____/_/ /_/ /_/ .___/\____/_/ /_/\___/_/ /_/\__/____/
*                        /_/
*/
void getTransformPosition(WrenVM* vm) {
    const Transform* t = wrenly::getForeignSlotPtr<Transform, 0>(vm);
    wrenly::setForeignSlotValue(vm, t->position);
}

void getTransformRotation(WrenVM* vm) {
    const Transform* t = wrenly::getForeignSlotPtr<Transform, 0>(vm);
    wrenly::setForeignSlotValue(vm, t->rotation);
}

void getTransformScale(WrenVM* vm) {
    const Transform* t = wrenly::getForeignSlotPtr<Transform, 0>(vm);
    wrenly::setForeignSlotValue(vm, t->scale);
}

}   // wren
}   // pg
