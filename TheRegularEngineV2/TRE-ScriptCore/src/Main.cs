using System;

public class Main
{
    public Main()
    {
        Console.WriteLine("Constructor");
    }

    public void Print()
    {
        Console.WriteLine("Test C#");
    }

    public void PrintCustom(string msg)
    {
        Console.WriteLine($"C#: {msg}");
    }
}