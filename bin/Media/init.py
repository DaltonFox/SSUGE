import ssuge
import math

ssuge.loadScript("..\\Media\\my_script.py")	
ssuge.createGroup("pyobjects")
terrain = ssuge.Terrain("pyobjects", "terrain")
bullets = ssuge.ItemContainer("pyobjects", "bullets")
tank = ssuge.PlayerTank("pyobjects", "tank", bullets)

pyramids = ssuge.ItemContainer("pyobjects", "pyramids")
pyramids.contents.append(ssuge.Pyramid("pyobjects", "pyramid1", 1100, 850))
pyramids.contents.append(ssuge.Pyramid("pyobjects", "pyramid2", -1100, -750))
pyramids.contents.append(ssuge.Pyramid("pyobjects", "pyramid3", -900, 750))
pyramids.contents.append(ssuge.Pyramid("pyobjects", "pyramid4", 700, 0))
pyramids.contents.append(ssuge.Pyramid("pyobjects", "pyramid5", 800, -950))
pyramids.contents.append(ssuge.Pyramid("pyobjects", "pyramid6", -200, -1050))
pyramids.contents.append(ssuge.Pyramid("pyobjects", "pyramid6", 200, 250))

pods = ssuge.ItemContainer("pyobjects", "pods")
for i in range(18):
    tank_pos = tank.getWorldPosition()
    if (ssuge.getRandomInt(0, 2)):
        x = ssuge.getRandomFloat(-900, 900)
        if x > 0:
            x += tank_pos[0]
        else:
            x -= tank_pos[0]
        if (ssuge.getRandomInt(0, 2)):
            y = tank_pos[2] + 900
        else:
            y = tank_pos[2] - 900
    else:
        y = ssuge.getRandomFloat(-900, 900)
        if y > 0:
            y += tank_pos[2]
        else:
            y -= tank_pos[2]
        if (ssuge.getRandomInt(0, 2)):
            x = tank_pos[0] + 900
        else:
            x = tank_pos[0] - 900
    
    pods.contents.append(ssuge.Pod("pyobjects", "pod" + str(i), x, y, tank))
