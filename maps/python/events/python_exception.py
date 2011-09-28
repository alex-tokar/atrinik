## @file
## Exception handling.
##
## This is basically an improved version of Python C API exception
## printing function; this version logs the message to the active server
## log file pointer (whether it's an actual file, stdout or stderr), as
## well as to all the online DMs.

import Atrinik, Markup, traceback

# Construct the exception string.
exception = "".join(traceback.format_exception(exc_type, exc_value, exc_traceback))

# Log the exception to the server log.
Atrinik.LOG(Atrinik.llevDebug, exception)

# Escape the markup in the exception message, and print it out to all online DMs.
exception = Markup.markup_escape(exception)
player = Atrinik.GetFirst("player")

while player:
	if player.ob.f_wiz or "dm" in player.cmd_permissions or "resetmap" in player.cmd_permissions:
		player.ob.Write(exception, Atrinik.COLOR_RED)

	player = player.next