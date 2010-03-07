Todo: 
-  merge trunk changes (rev. 2471/2472) for cmake files to playground and test.


Needed:
- create/update sim.pot and *.po
- install mo - files
- 'make dist'
- correct directories for linux (already fine?)
- FindQt3 needs a small change for win32 (http://www.cmake.org/Bug/bug.php?op=show&bugid=3514&pos=1)

Nice to have:
- pch support
- detect gcc visibility support (kde has a working check for)
- disallow in-source builds (I don't like in-source builds...)
