import java.io.*;
import java.util.*;

public class Main
{
	public static void Benchmark() throws FileNotFoundException
	{
		HashMap<String, Integer> m = new HashMap<String, Integer>();

		File file = new File("words.txt");
		Scanner input = new Scanner(file);
		
		input.useDelimiter(" ");

		while(input.hasNext())
		{
			String word = input.next();
			
			int count = m.getOrDefault(word, 0);
			
			m.put(word, count + 1);
		}
		
		input.close();
		
		System.out.println(m.size());

		int sum = 0;

		for(Map.Entry<String, Integer> kv : m.entrySet())
		{
			sum += kv.getValue();
		}

		System.out.println(sum);
	}

	public static void main(String[] args) throws FileNotFoundException
	{
		long startTime = System.currentTimeMillis();

		Benchmark();
		
		System.out.println(System.currentTimeMillis() - startTime);
	}
}
