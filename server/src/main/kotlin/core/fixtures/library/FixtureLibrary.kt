package core.fixtures.library

import util.FileUtils
import java.io.File
import java.nio.file.Files
import java.nio.file.Paths
import javax.xml.parsers.DocumentBuilderFactory

class FixtureLibrary(libraryDirectory: String) {

    private var libraryDir = libraryDirectory
    private var manufacturers = ArrayList<FixtureManufacturer>()
    private var models = HashMap<String, ArrayList<String>> ()

    fun createLibrary() {

        val exportDir = "$libraryDir/extracted"
        //First delete and recreate the files directory for extracted GDTFs
        val fileDir = File(exportDir)
        if(fileDir.exists()) {
            FileUtils.deleteDirectory(fileDir)
        }

        val path = Paths.get(exportDir);
        Files.createDirectory(path);

        manufacturers.clear()
        models.clear()

        //Iterate through all files in the library directory
        File("$libraryDir/gdtf").walk().forEach {
            if(it.name.endsWith(".gdtf", true)) {
               parseFile(it)
            }
        }

        //Iterate through extracted files and find manufacturer and model details in the respective description XML
        File("$libraryDir/extracted").walk().forEach {
            val xml = File(it.path + "/description.xml")
            if(xml.exists()) {
                parseXmlFile(xml)
            }
        }
    }

    private fun parseFile(file : File) {
        println("Parsing file ${file.name}")
        FileUtils.unzip(file.absolutePath, "$libraryDir/extracted/${file.nameWithoutExtension}")
    }

    private fun parseXmlFile(file : File) {
        val dbFactory = DocumentBuilderFactory.newInstance()
        val dBuilder = dbFactory.newDocumentBuilder()
        val doc = dBuilder.parse(file)

        val fTypeTag = doc.getElementsByTagName("FixtureType")
        if(fTypeTag.length>0) {
            val manufacturer = fTypeTag.item(0).attributes.getNamedItem("Manufacturer").nodeValue
            val modelName = fTypeTag.item(0).attributes.getNamedItem("Name").nodeValue
            val modelDescription = fTypeTag.item(0).attributes.getNamedItem("Description").nodeValue
            val modelThumbnail = fTypeTag.item(0).attributes.getNamedItem("Thumbnail").nodeValue
            val modes = ArrayList<FixtureMode>()

            val modesXML = doc.getElementsByTagName("DMXMode")

            for(m in 0 until modesXML.length) {
                modes.add(FixtureMode(modesXML.item(m).attributes.getNamedItem("Name").nodeValue))
            }

            val newManufacturer = FixtureManufacturer(manufacturer)

            if(!manufacturers.contains(newManufacturer))
            {
                manufacturers.add(newManufacturer)
            }

            if(!models.containsKey(manufacturer))
            {
                models[manufacturer] = ArrayList<String>()

                //modelList?.add(FixtureModel(newManufacturer, modelName, modelDescription, modelThumbnail, modes))
            }

            models[manufacturer]?.add(modelName)
            /*else
            {
                //val modelList = T_FixtureModels(ArrayList<T_FixtureModel>())
                //modelList.models.add(T_FixtureModel(modelName, manufacturer, modelDescription, modelThumbnail, modes))

                modelList
            }*/

            println(manufacturer + ": " + modelName)
        }
    }
//
//    public fun getFixtureManufacturers(): List<T_FixtureManufacturer>
//    {
//        return manufacturers.toList()
//    }
//
//    public fun getFixtureModels(manufacturer: String): T_FixtureModels
//    {
//        if(models.containsKey(manufacturer))
//        {
//            return models.get(manufacturer)!!
//        }
//        else
//        {
//            return T_FixtureModels(ArrayList<T_FixtureModel>())
//        }
//    }
}