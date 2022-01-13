package core

import core.fixtures.library.FixtureLibrary
import org.zapo.lumosmaxima.remote.APIService

object Core {
    private var fixtureLib = FixtureLibrary("fixturelib")
    private var api = APIService(2424)


    init {
        api.start()
        println("Core initialized.")
    }

    fun getFixtureLibrary() : FixtureLibrary {
        return fixtureLib
    }
}
