package core.fixtures.library

data class FixtureManufacturer(val name: String)

data class FixtureModel(val manufacturer: FixtureManufacturer,
                        val name: String,
                        val description: String,
                        val thumbnail: String,
                        val modes: ArrayList<FixtureMode>)

data class FixtureMode(val name: String)