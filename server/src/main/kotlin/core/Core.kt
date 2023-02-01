package core

import core.fixtures.library.FixtureLibrary


object Core {
    private var fixtureLib = FixtureLibrary("fixturelib")


    init {
        println("Core initialized.")
    }

    fun getFixtureLibrary() : FixtureLibrary {
        return fixtureLib
    }
}
