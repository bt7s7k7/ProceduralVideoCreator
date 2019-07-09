local size = Vector.new(1920,1080)

setDimensions(size:x(), size:y())
setLength(10)



function update(time) 
	tasks.rect(Vector.new(0,0), Vector.new(size:x(), mapTime(time, 0, 1) * size:y()), false, true, 255, 255, 0)
	tasks.rect(size:mul(0.5), Vector.new(size:x(), mapTime(time, 0.5, 1) * size:y()), true, true, 0, 255, 0)
	tasks.rect(size:mul(0.5), Vector.new(510,510):mul(mapTime(time, 1, 1)), true, false, 0, 0, 0)
	tasks.rect(size:mul(0.5), Vector.new(510,510):mul(mapTime(time, 1.25, 1)), true, false, 0, 0, 0)
	tasks.rect(size:mul(0.5), Vector.new(500,500):mul(mapTime(time, 1.5, 1)), true, true, 0, 0, 0)
	tasks.circle(size:mul(0.5), 400 * (mapTime(time, 2, 1)), false, 255, 255, 255)
	tasks.circle(size:mul(0.5), 400 * (mapTime(time, 2.5, 1)), true, 255, 255, 255)
end