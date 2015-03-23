## @file
## Script to execute when Nyhelobo the beholder dies.

me.FindObject(type = Type.SPAWN_POINT_INFO).owner.Destroy()
me.map.DrawInfo(me.x, me.y, "As soon as {} dies, a powerful shock wave hits the whole room, and the beholder's mind controlling equipment is destroyed.".format(me.name), color = COLOR_GREEN, distance = MAP_INFO_ALL)
