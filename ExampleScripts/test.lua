local size = Vector.new(1920,1080)

setDimensions(size:x(), size:y())
setLength(10)

local testImage = loadImage("testImage.png")

function update(time) 
	math.randomseed(time * 100)
	
	tasks.rect(Vector.new(0,0), Vector.new(size:x(), mapTime(time, 0, 1) * size:y()), false, true, 0.1, 0.1, 0.1)
	tasks.rect(size:mul(0.5), Vector.new(size:x(), mapTime(time, 0.5, 1) * size:y()), true, true, 0.2, 0.2, 0.2)
	tasks.rect(size:mul(0.5), Vector.new(510,510):mul(mapTime(time, 1, 1)), true, false, 0, 0, 0)
	tasks.rect(size:mul(0.5), Vector.new(510,510):mul(mapTime(time, 1.25, 1)), true, false, 0, 0, 0)
	tasks.rect(size:mul(0.5), Vector.new(500,500):mul(mapTime(time, 1.5, 1)), true, true, 0, 0, 0)
	tasks.circle(size:mul(0.5), 200 * (mapTime(time, 2, 1)), false, 1, 1, 1)
	for i=1,2 do 
		tasks.circle(size:mul(0.5):add(Vector.new(math.random(-20,20), math.random(-20,20))), (200 + math.random(-20,20)) * (mapTime(time, 2.1, 1)), false, 1, 1, 1)
	end
	tasks.line(size:mul(0.5), size:mul(0.5):add(Vector.fromAngle(time):mul(mapTime(time, 2.5, 1) * 200)), 1,1,1)

	tasks.text(Vector.new(size:x() / 2, size:y() * 0.1), 40, Vector.center, lerpText("", "PVC - Procedural Video Creator", mapTime(time, 3,1)), -1, 1,1,1)

	tasks.image(size:mul(0.5), Vector.new(1078,298), Vector.one:mul(mapTime(time, 4, 1)), true, testImage)
end