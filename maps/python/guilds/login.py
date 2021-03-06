## @file
## Activated on login map-wide event used in guild maps so players that
## are no longer members of the guild are kicked out.

from Guild import Guild

guild = Guild(GetOptions())

def main():
    if not guild.member_approved(activator.name):
        pl.DrawInfo("You have been removed from the guild while you were offline. Goodbye!", COLOR_RED)
        guild.member_kick(activator)
    elif activator.map.path[-7:] == "/oracle" and not guild.member_is_admin(activator.name):
        pl.DrawInfo("You have had administrator rights taken away while you were offline.", COLOR_RED)
        guild.member_kick(activator)

main()
