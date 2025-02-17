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
        String tempDir = "library/temp";

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
            /*try
            {
                ZipUtils.UnzipFile(gdtfFile, "");
            }*/
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

    private static void extractGDTF(File gdtfFile)
    {

    }
}
