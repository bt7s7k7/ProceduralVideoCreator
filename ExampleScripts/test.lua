local size = Vector.new(1920,1080)

setDimensions(size:x(), size:y())
setLength(10)



function update(time) 
	math.randomseed(time * 100)
	
	tasks.rect(Vector.new(0,0), Vector.new(size:x(), mapTime(time, 0, 1) * size:y()), false, true, 255, 255, 0)
	tasks.rect(size:mul(0.5), Vector.new(size:x(), mapTime(time, 0.5, 1) * size:y()), true, true, 0, 255, 0)
	tasks.rect(size:mul(0.5), Vector.new(510,510):mul(mapTime(time, 1, 1)), true, false, 0, 0, 0)
	tasks.rect(size:mul(0.5), Vector.new(510,510):mul(mapTime(time, 1.25, 1)), true, false, 0, 0, 0)
	tasks.rect(size:mul(0.5), Vector.new(500,500):mul(mapTime(time, 1.5, 1)), true, true, 0, 0, 0)
	tasks.circle(size:mul(0.5), 200 * (mapTime(time, 2, 1)), false, 255, 255, 255)
	for i=1,2 do 
		tasks.circle(size:mul(0.5):add(Vector.new(math.random(-20,20), math.random(-20,20))), (200 + math.random(-20,20)) * (mapTime(time, 2.1, 1)), false, 255, 255, 255)
	end

end