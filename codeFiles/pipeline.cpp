#include <iostream>
#include <list>


using namespace std;

int p=0;

void exec(list<char> &main_list){

	int t=0;
	int s=0;

	for(auto i=main_list.rbegin(); i!=main_list.rend();i++){
		switch(*i){
			case 'f':
				cout<<"f";
				p++;
				if(p<3)
					t=1;
				*i='d';	
				break;



			case 'd':
				cout<<"d";
				*i='e';	
				break;



			case 'e':
				cout<<"e";
				*i='m';	
				break;


			case 'm':
				cout<<"m";
				*i='w';	
				break;


			case 'w':
				cout<<"w";
				s=1;	
				break;

		}
		


	}

	if(t)
		main_list.push_front('f');
	if(s)
		main_list.pop_back();
	cout<<"\n";

}


int main() {
    
	list<char> main_list{'f'};
	
	while(!main_list.empty()){
		
		exec(main_list);
		

	}

    return 0;
}