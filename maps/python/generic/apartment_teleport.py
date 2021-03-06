## @file
## Generic apartment teleporter script.
##
## Used to teleport apartment owners to their apartment.

def main():
    apartment = apartments_info[GetOptions()]
    pinfo = activator.FindObject(archname = "player_info", name = apartment["tag"])

    # No apartment, teleport them back.
    if not pinfo:
        pl.DrawInfo("You don't own an apartment here!", COLOR_WHITE)
        activator.SetPosition(me.hp, me.sp)
    else:
        pinfo.race = me.map.path
        pinfo.last_sp = me.hp
        pinfo.last_grace = me.sp

        info = apartment["apartments"][pinfo.slaying]
        activator.TeleportTo(activator.map.GetPath(info["path"], True, activator.name), info["x"], info["y"])

exec(open(CreatePathname("/python/generic/apartments.py")).read())
main()
SetReturnValue(1)
