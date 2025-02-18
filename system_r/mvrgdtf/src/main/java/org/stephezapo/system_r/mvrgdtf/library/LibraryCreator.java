package org.stephezapo.system_r.mvrgdtf.library;

import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.net.URL;
import java.util.ArrayList;
import java.util.List;
import java.util.Objects;
import java.util.Set;
import java.util.concurrent.atomic.AtomicInteger;
import java.util.stream.Collectors;
import java.util.stream.Stream;
import java.util.zip.ZipEntry;
import java.util.zip.ZipFile;
import java.util.zip.ZipInputStream;

public class LibraryCreator implements Runnable
{
    private boolean running = true;
    private AtomicInteger progress = new AtomicInteger(0);
    private int totalFiles = 0;

    public LibraryCreator()
    {
        progress.set(0);
    }

    @Override
    public void run()
    {
        String libraryDir = "library";
        String tempDir = libraryDir + "/temp";

        try
        {
            ZipUtils.UnzipFile("./gdtf_share_library.zip", tempDir, "gdtf");
        }
        catch(IOException iex)
        {
            System.err.println("Could not unzip library archive.");
            iex.printStackTrace();
        }

        Set<String> gdtfFiles = listFiles(tempDir);
        totalFiles = gdtfFiles.size();

        if(totalFiles==0)
        {
            System.out.println("No GDTF files extracted. Done.");
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
                iex.printStackTrace();
            }
            progress.set((int)Math.round((100.0*filesDone)/(double)totalFiles));
        }
    }

    private static Set<String> listFiles(String dir)
    {
        return Stream.of(new File(dir).listFiles())
            .filter(file -> file.getName().endsWith(".gdtf"))
            .map(File::getName)
            .collect(Collectors.toSet());
    }

    private static String[] extractGdtfInfo(String gdtfFilePath)
    {
        File xmlFile = new File(gdtfFilePath);

        return new String[]{};
    }
}
