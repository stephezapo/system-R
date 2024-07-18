package org.stephezapo.system_r.core;

public class Core
{
    public static void main(String[] args)
    {
        System.out.println("Hello world!");
        System.out.println(TheLibrary.sayHello("Stephan"));
    }

    public static String sayHelloFromLibrary(String name)
    {
        return TheLibrary.sayHello(name);
    }
}