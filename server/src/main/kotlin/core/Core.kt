package core

import core.fixtures.library.FixtureLibrary
import org.zapo.lumosmaxima.remote.APIService

object Core {
    internal var fixtureLib = FixtureLibrary("fixturelib")
    internal var api = APIService(2424)


    init {
        api.start()
        println("Core initialized.")
    }

    fun getFixtureLibrary() : FixtureLibrary {
        return fixtureLib
    }
}
