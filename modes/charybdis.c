#include "stdinc.h"
#include "modules.h"
#include "client.h"
#include "ircd.h"
#include "chmode.h"
#include "channel.h"

struct module_modes ModuleModes;

static int
_modinit(void)
{
	ModuleModes.MODE_NOCTCP = cflag_add('C', chm_simple);
	if (ModuleModes.MODE_NOCTCP == 0)
		return -1;

	ModuleModes.MODE_REGONLY = cflag_add('r', chm_regonly);
	if (ModuleModes.MODE_REGONLY == 0)
		return -1;

	ModuleModes.MODE_NOCOLOR = cflag_add('c', chm_simple);
	if (ModuleModes.MODE_NOCOLOR == 0)
		return -1;

	ModuleModes.MODE_EXLIMIT = cflag_add('L', chm_staff);
	if (ModuleModes.MODE_EXLIMIT == 0)
		return -1;

	ModuleModes.MODE_PERMANENT = cflag_add('P', chm_staff);
	if (ModuleModes.MODE_PERMANENT == 0)
		return -1;

	ModuleModes.MODE_OPMODERATE = cflag_add('z', chm_simple);
	if (ModuleModes.MODE_OPMODERATE == 0)
		return -1;

	ModuleModes.MODE_FREEINVITE = cflag_add('g', chm_simple);
	if (ModuleModes.MODE_FREEINVITE == 0)
		return -1;

	ModuleModes.MODE_FREETARGET = cflag_add('F', chm_simple);
	if (ModuleModes.MODE_FREETARGET == 0)
		return -1;

	ModuleModes.MODE_DISFORWARD = cflag_add('Q', chm_simple);
	if (ModuleModes.MODE_DISFORWARD == 0)
		return -1;

	ModuleModes.CHFL_QUIET = cflag_add('q', chm_ban);
	if (ModuleModes.CHFL_QUIET == 0)
		return -1;

	ModuleModes.MODE_FORWARD = cflag_add('f', chm_forward);
	if (ModuleModes.MODE_FORWARD == 0)
		return -1;

	ModuleModes.MODE_THROTTLE = cflag_add('j', chm_throttle);
	if (ModuleModes.MODE_THROTTLE == 0)
		return -1;

	return 0;
}

static void
_moddeinit(void)
{
	cflag_orphan('C');
	ModuleModes.MODE_NOCTCP = 0;

	cflag_orphan('r');
	ModuleModes.MODE_REGONLY = 0;

	cflag_orphan('c');
	ModuleModes.MODE_NOCOLOR = 0;

	cflag_orphan('L');
	ModuleModes.MODE_EXLIMIT = 0;

	cflag_orphan('P');
	ModuleModes.MODE_PERMANENT = 0;

	cflag_orphan('z');
	ModuleModes.MODE_OPMODERATE = 0;

	cflag_orphan('g');
	ModuleModes.MODE_FREEINVITE = 0;

	cflag_orphan('F');
	ModuleModes.MODE_FREETARGET = 0;

	cflag_orphan('Q');
	ModuleModes.MODE_DISFORWARD = 0;

	cflag_orphan('q');
	ModuleModes.CHFL_QUIET = 0;

	cflag_orphan('f');
	ModuleModes.MODE_FORWARD = 0;

	cflag_orphan('j');
	ModuleModes.MODE_THROTTLE = 0;
}

DECLARE_MODULE_AV1(charybdis, _modinit, _moddeinit, NULL, NULL, NULL, "$charybdis$");
