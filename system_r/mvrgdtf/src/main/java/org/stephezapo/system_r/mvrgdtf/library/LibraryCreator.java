package org.stephezapo.system_r.mvrgdtf.library;

import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.net.URL;
import java.util.ArrayList;
import java.util.List;
import java.util.concurrent.atomic.AtomicInteger;
import java.util.zip.ZipEntry;
import java.util.zip.ZipFile;
import java.util.zip.ZipInputStream;

public class LibraryCreator implements Runnable
{
    private boolean running = true;
    private AtomicInteger progress = new AtomicInteger(0);

    public LibraryCreator()
    {
        progress.set(0);
    }

    @Override
    public void run()
    {
        ClassLoader classloader = Thread.currentThread().getContextClassLoader();
        InputStream is = classloader.getResourceAsStream("mvrgdtf/gdtf_share_library.zip");
        File outputDir = new File("library");



        try
        {
            byte[] buffer = new byte[1024];
            ZipInputStream zis = new ZipInputStream(is);
            ZipEntry zipEntry = zis.getNextEntry();

            List<ZipEntry> gdtfFiles = new ArrayList<>();

            while (zipEntry != null)
            {
                System.out.println(zipEntry.getName());
                if(zipEntry.getName().endsWith(".gdtf"))
                {
                    gdtfFiles.add(zipEntry);
                }
                // each zipEntry is a GDTF file (a zip file itself). Other filetypes are not allowed
                //extractGDTF(zipEntry.get)
                /*File newFile = newFile(outputDir, zipEntry);

                if (zipEntry.isDirectory())
                {
                    continue; // this should actually never happen, we only expect GDTF files in the root folder
                }
                else
                {
                    // fix for Windows-created archives
                    File parent = newFile.getParentFile();
                    if (!parent.isDirectory() && !parent.mkdirs())
                    {
                        throw new IOException("Failed to create directory " + parent);
                    }

                    // write file content
                    FileOutputStream fos = new FileOutputStream(newFile);
                    int len;
                    while ((len = zis.read(buffer)) > 0) {
                        fos.write(buffer, 0, len);
                    }
                    fos.close();
                }*/
                zipEntry = zis.getNextEntry();
            }

            zis.closeEntry();
            zis.close();


        }
        catch(Exception ex)
        {
            ex.printStackTrace();
        }

        File destDir = new File("src/main/resources/unzipTest");
    }

    private static File newFile(File destinationDir, ZipEntry zipEntry) throws IOException
    {
        File destFile = new File(destinationDir, zipEntry.getName());

        String destDirPath = destinationDir.getCanonicalPath();
        String destFilePath = destFile.getCanonicalPath();

        if (!destFilePath.startsWith(destDirPath + File.separator)) {
            throw new IOException("Entry is outside of the target dir: " + zipEntry.getName());
        }

        return destFile;
    }
}
