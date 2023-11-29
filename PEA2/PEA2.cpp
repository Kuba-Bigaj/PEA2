#include <iostream>
#include <fstream>
#include <string>
#include <conio.h>
#include <chrono>

class App {
	//data storage
	std::string loaded_file = "None";
	int** matrix = nullptr;
	int size = 0;

	//algorith-specific data storage
	int run_limit = 60;				//in seconds
	double coveted_precision = 1.1; //how bad can the acceptable solution be

	//SA
	std::string cooling_schedules[3] = { "Geometric", "Logarithmic", "Linear" };
	int chosen_cooling_schedule = 0;
	double cooling_coefficient = 0.999;
	//0 : 0.9999, 0.9999, 0.999
	//1 : 0.001, 0.001, 0.01
	//2 : 1e-6 , 1e-5, 5e-5

	//static UI helper functions
	static int create_sub_menu(std::string top_banner, std::string options[], std::string bot_banner, int number, int def_option) // displays a submenu with options, returns the number chosen
	{
		int chosen_option = def_option;
		while (true)
		{
			system("cls");
			std::cout << top_banner;
			for (int i = 0; i < number; i++)
			{
				if (i == chosen_option)
				{
					std::cout << "==>";
				}
				std::cout << "\t" << options[i] << "\n";
			}
			std::cout << bot_banner;
			switch (_getch())
			{
			case 72:
				chosen_option--;
				break;
			case 80:
				chosen_option++;
				break;
			case '\r':
				return chosen_option;
			default:
				break;
			}
			chosen_option = (chosen_option < 0 ? 0 : (chosen_option >= number ? number - 1 : chosen_option));
		}
	}

	static int create_adaptive_sub_menu(std::string top_banner, std::string options[], std::string bot_banner[], int number, int def_option) // displays a submenu with options, returns the number chosen
	{
		int chosen_option = def_option;
		while (true)
		{
			system("cls");
			std::cout << top_banner;
			for (int i = 0; i < number; i++)
			{
				if (i == chosen_option)
				{
					std::cout << "==>";
				}
				std::cout << "\t" << options[i] << "\n";
			}
			std::cout << bot_banner[chosen_option];
			switch (_getch())
			{
			case 72:
				chosen_option--;
				break;
			case 80:
				chosen_option++;
				break;
			case '\r':
				return chosen_option;
			default:
				break;
			}
			chosen_option = (chosen_option < 0 ? 0 : (chosen_option >= number ? number - 1 : chosen_option));
		}
	}

	static bool test_input_validity(std::string err_message)
	{
		if (std::cin.fail())
		{
			std::cout << err_message << "\n";
			std::cin.clear();
			std::cin.ignore(1000, '\n');
			system("pause");
			return false;
		}
		return true;
	}

	//data manipulation functions
	void dealloc_matrix() //deallocates the matrix, sets size to 0
	{
		if (!matrix)
		{
			size = 0;
			return;
		}
		for (int i = 0; i < size; i++)
		{
			delete matrix[i];
		}
		delete matrix;
		matrix = nullptr;
		size = 0;
	}

	void alloc_matrix(int s) //allocates the matrix
	{
		size = s;
		matrix = new int*[s];
		for (int i = 0; i < s; i++)
		{
			matrix[i] = new int[s];
		}
	}

	void show_data_legacy()
	{
		std::cout << "Loaded file: " << loaded_file << "\n";
		for (int i = 0; i < size; i++)
		{
			for (int j = 0; j < size; j++)
			{
				std::cout << matrix[i][j] << "\t";
			}
			std::cout << "\n";
		}
		system("pause");
	}

	void show_data()
	{
		std::cout << "Loaded file: " << loaded_file << "\nData size: " << size << "\n";
		system("pause");
	}

	void show_path(int* solution)
	{
		std::cout << solution[0];
		for (int i = 1; i < size; i++)
		{
			std::cout << " -> " << solution[i];
		}
		std::cout << "\n";
	}

	void read_data_from_file(std::string filename) //TODO adapt
	{
		std::ifstream file(filename);
		std::string line;

		if (!file.is_open())
		{
			std::cout << "File open error!\n";
			system("pause");
			return;
		}

		std::getline(file, line);	//title

		loaded_file = line.substr(6, line.size());

		std::getline(file, line);	//type
		std::getline(file, line);	//comment
		std::getline(file, line);	//dimension

		int f_size = std::stoi(line.substr(11, line.size() - 1));

		std::getline(file, line);	//edge type
		std::getline(file, line);	//edge format
		std::getline(file, line);	//edge section



		dealloc_matrix();

		size = f_size;

		alloc_matrix(size);

		int val;
		for (int i = 0; i < size; i++)
		{
			for (int j = 0; j < size; j++)
			{
				file >> val;
				if (file.fail())
				{
					std::cout << "Data read error!\n";
					break;
				}
				else
					matrix[i][j] = val;
			}
		}
		file.close();
		return;
	}

	void load_data()
	{
		std::string filename;
		std::cout << "Enter filename:\n";
		std::cin >> filename;
		if (test_input_validity("unsupported filename!\n"))
		{
			read_data_from_file(filename);
			std::cout << "Loaded data: \n";
			show_data();
		}
	}

	void copy_arr(int* src, int* dest)
	{
		for (int i = 0; i < size; i++)
		{
			dest[i] = src[i];
		}
	}

	//algorithms
	int path_len(int* solution)
	{
		int len = 0;
		for (int i = 1; i < size; i++)
		{
			len += matrix[solution[i - 1]][solution[i]];
		}
		return len;
	}

	void permutate(int* solution)
	{
		int one = rand() % size;
		int two = rand() % size;
		while (two == one)
			two = rand() % size;

		std::swap(solution[one], solution[two]);
	}

	void generate_neighbour(int* src, int* dest)
	{
		copy_arr(src, dest);
		permutate(dest);
	}

	double generate_random_double()
	{
		return (double)(rand() / RAND_MAX);
	}

	void cool(double& temp)
	{
		switch (chosen_cooling_schedule)
		{
		case 0:
			temp *= cooling_coefficient;
			break;
		case 1:
			temp /= (1 + cooling_coefficient * temp);
			break;
		case 2:
			temp -= cooling_coefficient;
			temp = temp < 0 ? 0 : temp;
			break;
		default:
			break;
		}
	}

	int lower_bound()
	{
		int res = 0, min;
		for (int i = 0; i < size; i++)
		{
			min = INT_MAX;
			for (int j = 0; j < size; j++)
			{
				if (i == j)
					continue;
				min = min < matrix[i][j] ? min : matrix[i][j];
			}
			res += min;
		}
		return res;
	}

	int* greedy()
	{
		int* solution = new int[size];
		bool* vis = new bool[size];
		bool is_done = false;
		int cur_vert, min, min_v;

		cur_vert = 0;
		for (int i = 0; i < size; i++)
			vis[i] = false;

		for (int c = 0; !is_done; c++)
		{
			solution[c] = cur_vert;
			vis[cur_vert] = true;

			is_done = true;
			min = INT_MAX;
			min_v = 0;

			for (int i = 0; i < size; i++)
			{
				if (vis[i])
					continue;

				is_done = false;

				if (matrix[cur_vert][i] < min)
				{
					min = matrix[cur_vert][i];
					min_v = i;
				}
			}
			cur_vert = min_v;
		}
		return solution;
	}

	void simulated_annealing()
	{
		if (size == 0)
		{
			std::cout << "No data loaded!\n";
			system("pause");
			return;
		}

		bool should_stop = false;
		int* s1 = greedy();
		int* s2 = new int[size];
		int l1, l2, lb = lower_bound();
		double temp = 0.3;
		generate_neighbour(s1, s2);
		int old = 0, cnt = 0;
		auto start = std::chrono::system_clock::now();
		std::chrono::duration<double> elapsed;

		while (!should_stop)
		{
			for (int i = 0; i < 8 * size; i++)
			{
				l1 = path_len(s1);
				l2 = path_len(s2);

				if (l1 > l2)
					copy_arr(s2, s1);
				else if (generate_random_double() < exp((-(l2 - l1)) / temp))
					copy_arr(s2, s1);

				generate_neighbour(s1, s2);
			}

			l1 = path_len(s1);

			//std::cout << l1 << "\t" << temp << "\n";

			cool(temp);

			elapsed = std::chrono::system_clock::now() - start;

			if (old == l1)
				cnt++;
			else
				cnt = 0;

			if (elapsed.count() > run_limit || l1 < coveted_precision * lb || (temp < 0.01 && cnt == 50))
				should_stop = true;
			old = l1;
		}
		std::cout << "Time elapsed: " << elapsed.count() << "s\nFinal solution: " << l1 << "\nTemperature:" << temp << "\ne^(-1/temp): " << exp(-1 / temp) << "\nPath:\n";
		show_path(s1);
		system("pause");
		delete s1;
		delete s2;
	}

	//UI functions

	std::string str_constraints()
	{
		return "Time constraint: " + std::to_string(run_limit) + "\nQuality constraint: " + std::to_string(coveted_precision) + "\n";
	}

	std::string str_cooling()
	{
		return "Cooling method: " + cooling_schedules[chosen_cooling_schedule] + "\nCooling coefficient: " + std::to_string(cooling_coefficient) + "\n";
	}

	void set_stop_conditions()
	{
		std::string options[3] = { "Set a time constraint", "Set a quality cconstraint", "Back" };
		int time_con;
		double qual_con;
		switch (create_sub_menu("Constraints\n", options, str_constraints(), 3, 0))
		{
		case 0:
			std::cout << "Enter a new time constraint (in seconds): ";
			std::cin >> time_con;
			if (test_input_validity("ERROR - not an integer!"))
				run_limit = time_con;
			break;
		case 1:
			std::cout << "Enter a new quality constraint (how much can the result be worse than lower bound): ";
			std::cin >> qual_con;
			if (test_input_validity("ERROR - not a double!"))
				coveted_precision = qual_con;
			break;
		case 2:
			return;
		default:
			return;
		}
		system("cls");
		std::cout << str_constraints();
		system("pause");
	}

	void set_cooling_schedule()
	{
		std::string hints[3] = { "Formula: T'=T*coeff\nCoefficients must be in (0, 1), preferebly very close to 1",
			"Formula: T'=T/(1+coeff*T)\nCoefficients must be in (0, 1), preferebly very close to 0",
			"Formula: T'=T-coeff\nCoefficients must be in (0, T_start), preferebly small" };
		chosen_cooling_schedule = create_adaptive_sub_menu(str_cooling(), cooling_schedules, hints, 3, 0);

		system("cls");
		std::cout << str_cooling();
		system("pause");

	}

	void set_cooling_coefficient()
	{
		system("cls");
		double new_coeff;
		std::cout << str_cooling();
		std::cout << "Input the new cooling coefficient:\n";
		std::cin >> new_coeff;
		if (test_input_validity("ERROR - not a double!"))
			cooling_coefficient = new_coeff;

		system("cls");
		std::cout << str_cooling();
		system("pause");
	}


public:
	void run()
	{
		std::string title = "MAIN MENU\n";
		std::string credits = "Kuba Bigaj 2023\n";
		std::string options[7] = { "Load data", "Show current data","Set stop conditions", "Simulated annealing", "[SA] Set cooling schedule","[SA] Set cooling coefficient", "Exit" };
		int chosen_option = 0;

		while (true)
		{
			chosen_option = create_sub_menu(title, options, credits, 7, chosen_option);

			switch (chosen_option)
			{
			case 0:
				load_data();
				break;
			case 1:
				show_data();
				break;
			case 2:
				set_stop_conditions();
				break;
			case 3:
				simulated_annealing();
				break;
			case 4:
				set_cooling_schedule();
				break;
			case 5:
				set_cooling_coefficient();
				break;
			case 6:
				return;
			default:
				break;
			}
		}
	}

	void debug()
	{
		read_data_from_file("ftv55.atsp");
		std::cout << path_len(greedy()) << "\n";
		simulated_annealing();
	}

	void run_tests()
	{

	}
};


int main(int argc, char *argv[])
{
	srand(time(NULL));
	App a;
	if (argc > 1)
	{
		if (strcmp(argv[1], "-t") == 0)
		{
			a.run_tests();
			return 0;
		}
	}
	a.run();
	//a.debug();
	return 0;
}

//TODO: write and read and calculate path from file