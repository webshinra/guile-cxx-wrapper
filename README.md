# A minimalist Guile c++ wrapper

Everyone wanting to use Guile with c++ encounter the same sadness : c
expressivity suck. Many compile-time check trivial in c++ shoud be
deffered to runtime.

Using guile as a data-support-dsl for a professional game engine, I
needed at least a minimal wrapper to ease my mind.

The idea is to :

 - Do not modify guile.
 - Do not use guile internal representation.
 - Maintain the runtime cost as low as possible.
 - Allow object pointer to be pass to scheme and back.
 - Improve argument type checking syntax.

This implementation isn't really elegant, but reduce overall
boilerplate-code nedded by guile.

I'm open to every suggestion/pull request concerning potential
improvements.

I will probably extend it over time, handeling more types and maybe
even more guile stuff.

# basic usage

To declare a c++ function callable form guile, the standard process
look like:

    static
    SCM
    create_dot(SCM x, SCM y)

you would then check arguments at runtime (no way past it : scheme
can pass you anything he want) :

    
     SCM_ASSERT (scm_is_integer(x),
                 x, 
                 SCM_ARG1, 
                 "create-dot");

and do the same thing for y.

With this type wrapper you will be able to be more precise in your
function declaration :

    static
    SCM
    create_dot(guile::wrap<int> x,
               guile::wrap<int> y)

Then, if you want to check if scheme give you what you wish, you just
have to call :

    x.check_as_arg<SCM_ARG1>("create-dot");

If needed, you could just convert it at will, like :

    std::pair<int, int> coordinates = {x,y};

without pain.

Please notice that guile::wrap types can take a second template
parameter, a boolean, like:

    guile::wrap<int, true>;

This parameter enable/disable the garbage collector protection.
When disabled (by default) the new operators are deleted, and only
stack allocation are allowed.

If it is set to true, It'll use RAII to protect/unprotect the object
during it's c++ lifetime.

The alternative syntax:

    guile::wrap<int, guile::GC_Protected>

and

    guile::wrap<int, guile::GC_NotProtected>

Are more explicit, and can be used when necessary.

Notice also that you can do (runtime) checked conversion using the
check() method, like:

    int X = x.check();

where x will be here checked with the scm_is_integer() predicate and
throw a bad_cast exception if something went wrong.  (the template
allow it to choose the correct one).

The file is structured to allow constructor template type deduction
when c++17 will be aviable.


thanks to Peter TB Brett (@peter-b on github) for his suggestion on
protecting guile object form GC in case of heap allocation.

This would probably have resulted in really nasty bugs in my far
future.
