package org.stephezapo.system_r.core.util;

import com.fasterxml.jackson.core.JsonProcessingException;
import com.fasterxml.jackson.databind.JsonMappingException;
import com.fasterxml.jackson.databind.ObjectMapper;
import java.io.BufferedReader;
import java.io.BufferedWriter;
import java.io.FileReader;
import java.io.FileWriter;
import java.io.IOException;
import java.io.Serializable;

public class Serializer
{
    public static boolean serialize(Serializable object, String fileName)
    {
        String jsonResult;

        try
        {
            ObjectMapper mapper = new ObjectMapper();
            jsonResult = mapper.writerWithDefaultPrettyPrinter()
                .writeValueAsString(object);
        }
        catch (JsonProcessingException e)
        {
            System.err.println("Could not process JSON");
            return false;
        }

        if(jsonResult.isEmpty())
        {
            return false;
        }

        write(jsonResult, fileName);
        return true;
    }

    public static Serializable deserialize(String fileName, Class<?> object)
    {
        String jsonString = read(fileName, object);

        if(jsonString.isEmpty())
        {
            return null;
        }

        try
        {
            ObjectMapper mapper = new ObjectMapper();
            return (Serializable) object.cast(mapper.readValue(jsonString, object));
        }
        catch (JsonMappingException e)
        {
            throw new RuntimeException(e);
        }
        catch (JsonProcessingException e)
        {
            throw new RuntimeException(e);
        }
    }

    private static String read(String fileName, Class<?> object)
    {
        try
        {
            BufferedReader reader = new BufferedReader(new FileReader(fileName));

            StringBuilder content = new StringBuilder();
            String line;

            while ((line = reader.readLine()) != null)
            {
                content.append(line);
                content.append(System.lineSeparator());
            }

            return content.toString();
        }
        catch(IOException iex)
        {
            System.err.println("Could not load file");
            return "";
        }
    }

    private static boolean write(String jsonString, String fileName)
    {
        try
        {
            BufferedWriter writer = new BufferedWriter(new FileWriter(fileName));
            writer.write(jsonString);
            writer.close();
            System.out.println("JSON File stored.");
            return true;
        }
        catch(IOException iex)
        {
            System.err.println("Could not write JSON String to file.");
            iex.printStackTrace();
            return false;
        }
    }
}
