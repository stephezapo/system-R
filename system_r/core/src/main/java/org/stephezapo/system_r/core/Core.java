package org.stephezapo.system_r.core;

public class Core
{
    private static DmxUniverse dmxUniverse = new DmxUniverse();

    public static void main(String[] args)
    {
        System.out.println("Hello world!");
        System.out.println(TheLibrary.sayHello("Stephan"));
    }

    public static String sayHelloFromLibrary(String name)
    {
        return TheLibrary.sayHello(name);
    }

    public static byte[] getDmxUniverse()
    {
        return dmxUniverse.getDmx();
    }
}