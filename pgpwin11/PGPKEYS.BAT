@echo off
pgp -kv > pgpk$tmp.$$1
ren pgpk$tmp.$$1 pgpk$tmp.$$$
