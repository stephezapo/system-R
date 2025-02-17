package org.stephezapo.system_r.mvrgdtf.library;

import java.io.File;
import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.io.IOException;
import java.util.Objects;
import java.util.zip.ZipEntry;
import java.util.zip.ZipInputStream;

public class ZipUtils
{
    public static void UnzipFile(String zipFile, String destDir, String filter) throws IOException
    {
        File destDirFile = new File(destDir);
        if(!destDirFile.exists())
        {
            destDirFile.mkdirs();
        }
        else
        {
            for (File file: Objects.requireNonNull(destDirFile.listFiles()))
            {
                file.delete();
            }
        }

        byte[] buffer = new byte[1024];
        ZipInputStream zis = new ZipInputStream(new FileInputStream(zipFile));
        ZipEntry zipEntry = zis.getNextEntry();
        while (zipEntry != null)
        {
            try
            {
                File newFile = newFile(destDirFile, zipEntry);
                if (zipEntry.isDirectory())
                {
                    if (!newFile.isDirectory() && !newFile.mkdirs())
                    {
                        throw new IOException("Failed to create directory " + newFile);
                    }
                }
                else if(filter.isEmpty() || zipEntry.getName().endsWith(filter))
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
                    while ((len = zis.read(buffer)) > 0)
                    {
                        fos.write(buffer, 0, len);
                    }
                    fos.close();
                }
            }
            catch(IOException iex)
            {
                // just skip file writing for now
            }

            zipEntry = zis.getNextEntry();
        }

        zis.closeEntry();
        zis.close();
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
