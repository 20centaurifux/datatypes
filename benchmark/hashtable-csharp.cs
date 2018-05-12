using System;
using System.Text;
using System.Collections.Generic;
using System.IO;
using System.Diagnostics;

namespace Benchmark
{
	class Tokenizer
	{
		public delegate void TokenHandler(string text);
		public event TokenHandler OnFound;

		private readonly StringBuilder _builder = new StringBuilder();

		public void Fill(byte[] bytes, int offset, int size)
		{
			_builder.Append(Encoding.UTF8.GetString(bytes, 0, size));

			var text = _builder.ToString();

			int left = 0, right = 0;

			while(left < text.Length)
			{
				right = text.IndexOf(' ', left);

				if(right == -1)
				{
				    break;
				}

				OnFound?.Invoke(text.Substring(left, right - left));

				left = ++right;
			}

			if(left > 0)
			{
				_builder.Clear();

				if(left < text.Length)
				{
					_builder.Append(text.Substring(left));
				}
			}
		}

		public void Flush()
		{
			var text = _builder.ToString();

			if(!string.IsNullOrWhiteSpace(text))
			{
				OnFound?.Invoke(text);
			}
		}
	}

	class Program
	{
		static void Benchmark()
		{
			var t = new Tokenizer();
			var m = new Dictionary<string, int>();

			t.OnFound += (string word) =>
			{
				if(m.ContainsKey(word))
				{
					m[word]++;
				}
				else
				{
					m[word] = 1;
				}
			};

			using(var stream = File.OpenRead("words.txt"))
			{
				var bytes = new byte[81920];
				int read;

				do
				{
					read = stream.Read(bytes, 0, 81920);

					if(read > 0)
					{
						t.Fill(bytes, 0, read);
					}
				} while(read > 0);

				t.Flush();

				Console.WriteLine(m.Count);

				int sum = 0;

				foreach(var kv in m)
				{
					sum += kv.Value;
				}

				Console.WriteLine(sum);
			}
		}

		static void Main(string[] args)
		{
			var ws = Stopwatch.StartNew();

			Benchmark();

			Console.WriteLine(ws.Elapsed);
		}
	}
}
