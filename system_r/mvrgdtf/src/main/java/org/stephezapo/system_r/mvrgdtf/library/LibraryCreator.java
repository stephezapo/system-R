package org.stephezapo.system_r.mvrgdtf.library;

import generated.DMXChannel;
import generated.DMXMode;
import generated.FixtureType;
import generated.GDTF;
import jakarta.xml.bind.JAXBContext;
import jakarta.xml.bind.JAXBException;
import java.io.File;
import java.io.FileNotFoundException;
import java.io.FileReader;
import java.io.IOException;
import java.nio.file.Files;
import java.util.Objects;
import java.util.Set;
import java.util.concurrent.atomic.AtomicInteger;
import java.util.stream.Collectors;
import java.util.stream.Stream;
import org.stephezapo.system_r.api.fixture.library.FixtureTypeInfo;
import org.stephezapo.system_r.api.fixture.library.FixtureTypeMode;
import org.stephezapo.system_r.api.fixture.library.LibraryInfo;

public class LibraryCreator implements Runnable
{
    private boolean running = true;
    private AtomicInteger progress = new AtomicInteger(0);
    private int totalFiles = 0;
    private LibraryInfo data;

    public LibraryCreator(LibraryInfo data)
    {
        this.data = data;
        progress.set(0);
    }

    @Override
    public void run()
    {
        running = true;
        String libraryDir = "library";
        String tempDir = libraryDir + "/temp";

        try
        {
            ZipUtils.UnzipFile("./gdtf_share_library.zip", tempDir, "gdtf");
        }
        catch(IOException iex)
        {
            System.err.println("Could not unzip library archive.");
            running = false;
            iex.printStackTrace();
        }

        Set<String> gdtfFiles = listFiles(tempDir);
        totalFiles = gdtfFiles.size();

        if(totalFiles==0)
        {
            System.out.println("No GDTF files extracted. Done.");
            running = false;
            return;
        }

        int filesDone = 0;
        for(String gdtfFile : gdtfFiles)
        {
            filesDone += 1;
            try
            {
                String fileName = gdtfFile.substring(0, gdtfFile.lastIndexOf(".gdtf"));
                ZipUtils.UnzipFile(tempDir + "/" + gdtfFile, libraryDir + "/" + fileName, "");

                // Parse the GDTF's description.xml
                extractGdtfInfo(libraryDir + "/" + fileName);

                System.out.println("Unzipped and extracted " + gdtfFile + ".");
            }
            catch(IOException iex)
            {
                System.out.println("Could not unzip " + gdtfFile + ". Skipping.");
                running = false;
                iex.printStackTrace();
            }
            progress.set((int)Math.round((99.0*filesDone)/(double)totalFiles));
        }

        // clean and remove the temp folder
        File tempDirFile = new File(tempDir);
        for (File file: Objects.requireNonNull(tempDirFile.listFiles()))
        {
            file.delete();
        }
        try
        {
            Files.delete(tempDirFile.toPath());
        }
        catch (IOException e)
        {
            System.err.println("Could not delete temporary directory.");
            running = false;
            e.printStackTrace();
        }

        progress.set(100);
        running = false;
        System.out.println("Library extraction done.");
    }

    public short getProgress()
    {
        return (short)progress.get();
    }

    public boolean isRunning()
    {
        return running;
    }

    public LibraryInfo getLibraryData()
    {
        return data;
    }

    private static Set<String> listFiles(String dir)
    {
        return Stream.of(new File(dir).listFiles())
            .map(File::getName)
            .filter(name -> name.endsWith(".gdtf"))
            .collect(Collectors.toSet());
    }

    private String[] extractGdtfInfo(String gdtfFilePath)
    {
        try
        {
            JAXBContext context = JAXBContext.newInstance(GDTF.class);
            GDTF gdtf = (GDTF) context.createUnmarshaller().
                unmarshal(new FileReader(gdtfFilePath + "/description.xml"));

            FixtureType type = gdtf.getFixtureType();
            FixtureTypeInfo info = new FixtureTypeInfo(type.getManufacturer(), type.getName(),
                String.valueOf(gdtf.getDataVersion()), gdtfFilePath);
            for(DMXMode dmxMode : type.getDMXModes().getDMXMode())
            {
                // we need to go through all channel functions to get the channel count
                short maximum = 0;

                for(DMXChannel channel : dmxMode.getDMXChannels().getDMXChannel())
                {
                    // Offset is defined by channel numbers separated by a comma
                    String[] offsetElements = channel.getOffset().split(",");
                    for(String s : offsetElements)
                    {
                        try
                        {
                            short channelNumber = Short.parseShort(s);
                            if(channelNumber>maximum)
                            {
                                maximum = channelNumber;
                            }
                        }
                        catch(NumberFormatException nfex)
                        {
                            // ignore for now
                        }
                    }
                }

                FixtureTypeMode
                    mode = new FixtureTypeMode(dmxMode.getName(), maximum);
                info.addMode(mode);
            }

            data.addFixtureTypeInfo(info);
        }
        catch(JAXBException ex)
        {
            System.err.println("Could not parse GDTF description.xml. Skipping.");
        }
        catch (FileNotFoundException e)
        {
            System.err.println("Could not find GDTF description.xml. Skipping.");
        }

        return new String[]{};
    }
}
