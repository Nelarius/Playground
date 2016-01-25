
import "builtin/math" for Math
import "builtin/vector" for Vec2, Vec3, createVec2
import "builtin/quaternion" for Quat
import "builtin/event" for EventManager
import "builtin/mouse" for Mouse
import "builtin/component" for Transform

var activate = Fn.new{
    EventManager.listenToKeyPressed(entity, "KeyW")
    EventManager.listenToKeyPressed(entity, "KeyA")
    EventManager.listenToKeyPressed(entity, "KeyS")
    EventManager.listenToKeyPressed(entity, "KeyD")
    EventManager.listenToMousePressed(entity, "Left")
}

var deactivate = Fn.new{
    //
}

var onKeyDown = Fn.new { |key|
    //
}

var speed = 10.0

var pos = Vec3.new( 0.0, 0.0, 10.0 )
var scale = Vec3.new( 1.0, 1.0, 1.0 )
var rot = Quat.new( 0.0, 0.0, 0.0, 1.0 )

var xAxis = Vec3.new(1.0, 0.0, 0.0)
var yAxis = Vec3.new(0.0, 1.0, 0.0)
var zAxis = Vec3.new(0.0, 0.0, -1.0)

var timeDelta = 0.016

var onKeyPressed = Fn.new { |key|
    if (key == "KeyW") {
        pos.z = pos.z - speed * timeDelta
    } else if (key == "KeyA") {
        pos.x = pos.x - speed * timeDelta
    } else if (key == "KeyS") {
        pos.z = pos.z + speed * timeDelta
    } else if (key == "KeyD") {
        pos.x = pos.x + speed * timeDelta
    }
    entity.transform = Transform.new(pos, rot, scale)
}

var onMousePressed = Fn.new { |button|
    var ndcx = Mouse.x
    var ndcy = Mouse.y
    ndcx = ndcx * 2.0 / 1200.0 - 1.0
    ndcy = 1.0 - ndcy * 2.0 / 800.0
    //System.print("ndcx: %(ndcx), ndcy: %(ndcy)")
}

var onKeyUp = Fn.new { |key|
    //
}

var update = Fn.new { | dt |
    timeDelta = dt
}
