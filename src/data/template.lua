

-- this gets called just after the component is assigned
function activate()
end

-- accumulated time
t = 0.0
-- angular velocity
av = 0.5
r = 8.0

-- this gets called in the update loop
function update( dt )
    if entity:hasTransform() then
        t = t + dt
        q = pg.Quaternion( 0.0, 0.0, math.sin( 10.0*dt ), math.cos( 10.0*dt ) )
        --entity.transform.position = pg.Vector3f( r*math.cos( t*av ), 0.0, r*math.sin( t*av ) )
        entity.transform.rotation = pg.Quaternion( 0.0, 0.0, math.sin( 0.2 ), math.cos( 0.2 ) )
    end
end

--  this gets called just before the component is removed
function deactivate()
    print("Script gonna hang up now!")
end
