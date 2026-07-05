# Postal3-2009-fix

### Notes 
- This is for the Postal III (Sep 29, 2009 prototype) port on Xbox 360.
- Fixes a very long hang when loading into maps.
- Fixes multiple crashes related to particles.
- Fixes issues in server_360.dll with interactable objects.
- Fixes multiple crashes when loading into a map.
- Fixes a crash when respawning.
- I've attempted to comment and explain what each patch is for.
- I've left my failed, or test patches in commented out.

### Issues
- Unfortunately, the particles have to be removed, and some jank things I had to patch.
- Something also seems to call KeBugCheck in Engine_360.dll sometimes.

### Credits
- Heaventh, creating this patch.
- Gonzo, helping test and finding issues. 

**If you crash, log it, and submit it in the issues page. I'll attempt to patch it**\
***- MAPS THAT REQUIRE 1GB OF RAM WILL CRASH WITHOUT IT! -***
