#$Id: dataSubselectorLib.py,v 1.2 2008/02/22 00:27:43 golpa Exp $
def generate(env, **kw):
    if not kw.get('depsOnly',0):
        env.Tool('addLibrary', library = ['dataSubselector'])
    env.Tool('tipLib')
    env.Tool('astroLib')
    env.Tool('st_facilitiesLib')
    env.Tool('facilitiesLib')
    env.Tool('evtbinLib')

def exists(env):
    return 1
