#include "Header.h"
#include "MainWindow.h"

int main(int argc, char* argv[]){
	int to_quit = 0;
	std::thread helper_thread(helper, &to_quit);

	int xDim = 1600;
	int yDim = 900;
	MainWindow(xDim, yDim);
	to_quit = 1;

	helper_thread.join();
	return EXIT_SUCCESS;
}

void helper(int* to_quit) {
	cout << "print help" << endl;
	while (!(*to_quit)) {
		std::string s;
		cin >> s;
		if (s.compare("help") == 0) {
			cout << "Controls:" <<
				"  move with wasd and arrows" << endl <<
				"  q - decrease zoom" << endl <<
				"  e - increase zoom" << endl <<
				"  n - decrease collor coefficient" << endl <<
				"  m - increase collor coefficient" << endl <<
				"  f - decrease details" << endl <<
				"  r - increase details(much slower!)" << endl <<
				"  x - change color mode" << endl << endl;
		}
		SDL_Delay(100);
	}
}