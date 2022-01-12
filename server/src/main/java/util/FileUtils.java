package util;

import java.io.*;
import java.nio.file.*;
import java.util.Enumeration;
import java.util.zip.ZipEntry;
import java.util.zip.ZipFile;


public class FileUtils {

    public static void unzip(String zipFile, String destDir) {
        //Open the file

        try(ZipFile file = new ZipFile(zipFile))
        {
            FileSystem fileSystem = FileSystems.getDefault();
            //Get file entries
            Enumeration<? extends ZipEntry> entries = file.entries();

            //We will unzip files in this folder
            Files.createDirectory(fileSystem.getPath(destDir));

            //Iterate over entries
            while (entries.hasMoreElements())
            {
                ZipEntry entry = entries.nextElement();

                //If directory then create a new directory in uncompressed folder
                if (entry.isDirectory())
                {
                    Files.createDirectories(fileSystem.getPath(destDir + "/" + entry.getName()));
                }
                //Else create the file
                else
                {
                    /*First check if the directory is already existing
                    * (Sometimes files in ZIP are listed before their parent directories are listed and created)
                    */
                    if(entry.getName().contains("/"))
                    {
                        File parentFolder = new File(destDir + "/" + entry.getName().substring(0, entry.getName().lastIndexOf("/")));
                        if(!parentFolder.exists())
                        {
                            parentFolder.mkdirs();
                        }
                    }

                    InputStream is = file.getInputStream(entry);
                    BufferedInputStream bis = new BufferedInputStream(is);
                    String uncompressedFileName = destDir + "/" + entry.getName();
                    Path uncompressedFilePath = fileSystem.getPath(uncompressedFileName);
                    Files.createFile(uncompressedFilePath);
                    FileOutputStream fileOutput = new FileOutputStream(uncompressedFileName);

                    byte[] buf = new byte[4096];
                    int r;
                    while ((r = is.read(buf)) != -1) {
                        fileOutput.write(buf, 0, r);
                    }
                    fileOutput.close();
                    is.close();
                }
            }
        }
        catch(IOException e)
        {
            e.printStackTrace();
        }
    }


    public static boolean deleteDirectory(File dir) {
        if (dir.isDirectory()) {
            File[] children = dir.listFiles();
            for (int i = 0; i < children.length; i++) {
                boolean success = deleteDirectory(children[i]);
                if (!success) {
                    return false;
                }
            }
        }

        return dir.delete();
    }
}
