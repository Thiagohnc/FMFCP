#include <iostream>
#include <string>
#include <cstdlib>
#include <unistd.h>
#include <cstdio>
#include <cstring>
#include <sys/wait.h>
#include <ctime>
#include <iomanip>
#include <termios.h>

using namespace std;

int getch(){
    int ch;
    struct termios t_old, t_new;

    tcgetattr(STDIN_FILENO, &t_old);
    t_new = t_old;
    t_new.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &t_new);
    ch = getchar();

    tcsetattr(STDIN_FILENO, TCSANOW, &t_old);
    return ch;
}

string getpass(const char *prompt, bool show_asterisk=true){
  const char BACKSPACE=127;
  const char RETURN=10;

  string password;
  unsigned char ch=0;

  cout <<prompt<<endl;

  while((ch=getch())!=RETURN)
    {
       if(ch==BACKSPACE)
         {
            if(password.length()!=0)
              {
                 if(show_asterisk)
                 cout <<"\b \b";
                 password.resize(password.length()-1);
              }
         }
       else
         {
             password+=ch;
             if(show_asterisk)
                 cout <<'*';
         }
    }
  cout <<endl;
  return password;
}
	
string constroi_cmd_send(string& src_host_password, string& src_usr_name, string& src_host, string& src_file_dir, string& file_name, string& port){
	return "sshpass -p \"" + src_host_password + "\" ssh " + src_usr_name +"@"+ src_host + " \"tar -c -C "+ src_file_dir + " " + file_name + " | pigz -i9 | nc -w 1 -l " + port + " > /dev/null 2>&1 & \" ";	 
}

string constroi_cmd_recv(string& targ_host_password, string& src_usr_name, string& src_host, string& targ_file_dir, string& port){
	return "sshpass -p \"" + targ_host_password + "\" ssh -n " + src_usr_name + "@" + src_host + " \"cd " + targ_file_dir + "; nc " + src_host + " " + port + " | pigz -d | tar xvf - \" ";
}

string constroi_cmd_md5(string& host_password,string& usr_name,string& host,string file_path){
	
	return "sshpass -p \"" + host_password + "\" ssh " + usr_name +"@"+ host + " \"md5sum " + file_path+" \" ";
}

string constroi_cmd_get_filesize(string& src_host_password,string& src_usr_name,string& src_host,string& src_file_path){
	return "sshpass -p \"" + src_host_password + "\" ssh "+ src_usr_name +"@"+ src_host + " \"du -sb " + src_file_path + " | awk '{print \\$1}' \" ";
}

int main(int argc, char** argv){
	
	long long int file_size;
	string src_usr_name;
	string src_file_path;
	string file_name;
	string src_host;
	string src_file_dir;
	
	string targ_usr_name;
	string targ_file_dir;
	string targ_host;
	
	string src_info;
	string targ_info;
	
	string port("1234");
	
	string cmd_send;
	string cmd_recv;
	
	string cmd_src_md5;
	string cmd_targ_md5;
	
	string cmd_get_file_size;
	
	time_t time_init;
	time_t time_end;
	
	src_info= argv[1];
	targ_info= argv[2];
	
	src_host       = src_info.substr(src_info.find('@') + 1, src_info.find(':') - src_info.find('@') - 1);
	src_usr_name   = src_info.substr(0,src_info.find('@'));
	
	src_file_path  = src_info.substr(src_info.find(':') + 1);
	src_file_dir   = src_file_path.substr(0,src_file_path.find_last_of({'/', '\\'}));
	file_name	   = src_info.substr(src_info.find_last_of({'/', '\\'}) + 1);
	
	
	targ_host      = targ_info.substr(targ_info.find('@') + 1, targ_info.find(':') - targ_info.find('@') - 1);
	targ_usr_name  = targ_info.substr(0,targ_info.find('@'));
	targ_file_dir  = targ_info.substr(targ_info.find(':') + 1);
	
	//Le a senha do source host
	string src_host_password  = getpass("Digite a senha do source host: ",true);
	
	//Le a senha do target host
	string targ_host_password = getpass("Digite a senha do target host: ",true);	
	
	//Constroi o cmd de send
	cmd_send=constroi_cmd_send(src_host_password,src_usr_name,src_host,src_file_dir,file_name, port);
	
	//Constroi o cmd de recv
	cmd_recv =constroi_cmd_recv(targ_host_password,src_usr_name,src_host,targ_file_dir,port);
		
	//Constroi cmds de md5 do source e target
	cmd_src_md5=constroi_cmd_md5(src_host_password,src_usr_name,src_host,src_file_path);
	cmd_targ_md5=constroi_cmd_md5(targ_host_password,targ_usr_name,targ_host,targ_file_dir + "/" + file_name  );
	
	//Constroi o cmd de get file size
	cmd_get_file_size=constroi_cmd_get_filesize(src_host_password,src_usr_name,src_host,src_file_path);
	
	/*	 
		Relizando MD5 no source host
	*/
	cout <<"Fazendo ssh para calcular o md5 no source host: " + src_host << endl; 
	char buffer[1000];
	try{
		FILE * f=popen(cmd_src_md5.c_str(),"r");
		if(f==NULL) {
			throw "Error ao calcular o md5 do arquivo no source host; Verificar se o arquivo existe; Abortando o programa";
		} 
		fscanf(f,"%s",buffer);
		fclose(f);
	}
	catch(const char* e){
		cout << e << endl;
		exit(1);
	}
	
	/*	 
		Calculando tamanho do arquivo
	*/
	char buffer2[1000];		
	cout <<"Fazendo ssh para calcular o tamanho do arquivo no source host: " + src_host << endl;	
	try{
		FILE * f2=popen(cmd_get_file_size.c_str(),"r");
		if(f2==NULL){
			throw "Error ao calcular o tamanho do arquivo no source host; Verificar se o arquivo existe; Abortando o programa";
		}
		fscanf(f2,"%s",buffer2);
		fclose(f2);	
	}
	catch(const char* e){
		cout << e << endl;
		exit(1);
	}
	file_size=stoll(buffer2);		
	
	/*	 
		Iniciando transferencia do arquivo
	*/
	time(&time_init);
	cout <<"Fazendo ssh para iniciar a transferencia do arquivo do source host: " + src_host << endl;		
	try{
		if(system(cmd_send.c_str())){
			throw "Erro no comando de envio do arquivo; Abortando o programa";
		}
	}
	catch(const char* e){
		cout << e << endl;
		exit(1);
	}
	/*	 
		Iniciando recebimento do arquivo
	*/
		
	cout <<"Fazendo ssh com o destino para aprovar a transferencia para o target host: " + targ_host << endl;
	try{
		if(system(cmd_recv.c_str())){
			throw "Erro no comando de recebimento do arquivo; Abortando o programa";
		}
	}
	catch(const char* e){
		cout << e << endl;
		exit(1);
	}
	time(&time_end);
		
	/*	 
		Relizando MD5 no target host
	*/
	char buffer3[1000];
	cout <<"Fazendo ssh para calcular o md5 no target host: " + targ_host << endl;
	try{
		FILE * f3=popen(cmd_targ_md5.c_str(),"r");
		if(f3==NULL){
			throw "Error ao calcular o MD5 do arquivo no target host; Verificar se o arquivo existe; Abortando o programa";
		}
		fscanf(f3,"%s",buffer3);
		fclose(f3);
	}
	catch(const char* e){
		cout << e << endl;
		exit(1);
	}
	/*	 
		Comparando o MD5
	*/
	if(strcmp(buffer,buffer3)==0){
		double send_time = difftime(time_end,time_init);
		double velo=((double)file_size/(1<<20))/send_time;		
		cout << "Sucesso no envio do arquivo: " + file_name << endl;		
		cout << fixed << setprecision(2);
		cout << "O tempo de transferencia foi de: " << (send_time<60?send_time:send_time/60.0) << (send_time < 60?" segundos":" minutos") << endl;	
		cout <<  "A velocidade media de transferencia foi de: "<< velo << " MB/s" << endl;
		
	}
	else{
		cout << "A Verificacao do MD5 Falhou; Erro no envio do arquivo: " + file_name + "; Abortando o programa;" << endl;
		exit(1);
	}		
	
	return 0;	
	

}
