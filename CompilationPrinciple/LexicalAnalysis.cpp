#include "iostream"
#include "string"
#include "cstring"
#include "algorithm"
#include "map"
#include "vector"
#include "fstream"
using namespace std;

//���ڱ�֤ÿ�ε��ʵ����ͺ�����
struct Words {
	int type;
	string word;
	void printf() {
		cout << "(" << type << ", " << word << ")  ";
	}
};

//����map������ֱ��ƥ�䱣���֡���������ָ���
map<string, int> findMap;
//����������
vector<vector<Words>>  result;

/*��ʼ�����������ɲ���map*/
void init() {
	//1. c���Ա�����(ע��main���Ǳ�����)
	string keyword[] = { "if","else","while","do","int","float","double","return","const","void",
		"continue","break","char","signed","enum","long","switch","case","auto","unsigned" };
	for (string s : keyword) {
		//�������map��
		findMap[s] = 1;
	}
	//2. ��ʶ�� ƥ�����<��ĸ>|<�»���>(<��ĸ>|<�»���>|<����>) (�����ǹؼ���) -> ��Ӧ��� 2

	//3. ���� �������ֳ���(����/С��)���ַ�����(�������м�һ���ַ�:'a')��
	//        �ַ���(c�����ṩ���ַ�������û�ṩ�ַ������ͣ�˫�����м��ַ���:"") -> ��Ӧ��� 3

	//4. ����� (����ַ���ɵ������ʵ�ʴ���ʱ����ƥ�䵽��һ���ַ����ڿ���һ���Ƿ�������)
	string operationSymbol[] = { ".","->","!","~","++","--","+","-","&","/","%","<<",">>",">","<","<=",
		">=","==","!=","^","|","&&","||","?",":","=","+=","-=","*=","/=","&=","%=","^=" ,"|=","<<=,",">>=" };
	for (string s : operationSymbol) {
		findMap[s] = 4;
	}

	//5. �ָ���
	string separator[] = { "(",")","[","]","{","}",",",";","#"};
	for (string s : separator) {
		findMap[s] = 5;
	}
	//6. �쳣���
}

/*�����ascii���ַ�*/
void handleNoAsciiSymble(string& str, int& i, int len, vector<Words>& vt) {
	string s = "";
	while (i < len && (!isascii(str[i]))) {
		s += str[i++];
	}
	Words w = { 6,s };
	vt.push_back(w);
}

/*�������ֻ��ʶ��*/
void handleKeywordOrIdentifier(string& str, int& i, int len, vector<Words>& vt) {
	string s = "";
	while (i < len && (isalpha(str[i]) || str[i] == '_' || isdigit(str[i]))) {
		s += str[i];
		++i;
	}
	if (findMap.find(s) != findMap.end()) {
		//������
		Words w = { 1,s };
		vt.push_back(w);
	}
	else {
		//��ʶ��
		Words w = { 2,s };
		vt.push_back(w);
	}
}

/*�������ֳ���*/
void handleDigitConstant(string& str, int& i, int len, vector<Words>& vt) {
	string s = "";
	//���ֵ�С���������
	int dotNum = 0;
	while (i < len && (isdigit(str[i]) || str[i] == '.')) {
		if (str[i] == '.') {
			++dotNum;
		}
		s += str[i++];
	}
	Words w;
	if (dotNum > 1) {
		//�����������ϵ�С���� -> �쳣�ֶ�
		w = { 6,s };
	}
	else {
		w = { 3,s };
	}
	vt.push_back(w);
}

/*�����ַ�����*/
void handleCharacterConstant(string& str, int& i, int len, vector<Words>& vt) {
	string s = "\'";
	++i;
	Words w;
	//�������'a'��ʽ -> �쳣����
	if (i + 1 > len || str[i + 1] != '\'') {
		while (i < len) {
			s += str[i++];
		}
		w = { 6,s };
	}
	else {
		s += str[i++];
		s += str[i++];
		w = { 3,s };
	}
	vt.push_back(w);
}

/*�����ַ�������*/
void handleStringConstant(string& str, int& i, int len, vector<Words>& vt) {
	string s = "\"";
	++i;
	while (i < len && str[i] != '\"') {
		s += str[i++];
	}
	Words w;
	//��βû��" -> �쳣����
	if (i >= len || str[i] != '\"') {
		w = { 6,s };
	}
	else {
		s += str[i++];
		w = { 3,s };
	}
	vt.push_back(w);
}

/*���������ַ�*/
void handleOtherSymbol(string& str, int& i, int len, vector<Words>& vt) {
	Words w;
	string s = "";
	s += str[i];
	map<string, int>::iterator item = findMap.find(s);
	//�Ƿ��ַ� -> �쳣�ֶ�
	if (item == findMap.end()) {
		w = { 6,s };
	}
	//�ָ���
	else if (item->second == 5) {
		w = { 5,s };
	}
	//����ע�͵Ĵ��� -> ����һ���ж���ע��
	// "/" -�� "//"
	else if (s == "/" && i + 1 < len && str[i + 1] == '/') {
		++i;
		while (i < len) {
			s += str[i++];
		}
		w = { 7,s };
	}
	//����� ��ЩҪ��������
	else {
		// "+"  -��  "++" / "+="
		if (s == "+") {
			if (i + 1 < len && str[i + 1] == '+' || str[i + 1] == '=') {
				s += str[++i];
			}
		}
		//"-"  -��  "->" / "--" / "-="
		else if (s == "-") {
			if (i + 1 < len && str[i + 1] == '>' || str[i + 1] == '-' || str[i + 1] == '=') {
				s += str[++i];
			}
		}
		//"*""/""%""^""!""="  -�� "*=""/=""%=""^=""!=""=="
		else if (s == "*" || s == "/" || s == "%" || s == "|" || s == "!" || s == "=") {
			if (i + 1 < len && str[i + 1] == '=') {
				s += str[++i];
			}
		}
		//"&""|"  -��  "&&""||" / "&=""|="
		else if (s == "&" || s == "|") {
			if (i + 1 < len && (str[i + 1] == s[0] || str[i + 1] == '=')) {
				s += str[++i];
			}
		}
		//">""<"  -�� ">=""<=" / ">>""<<"  -�� ">>=""<<="
		else if (s == ">" || s == "<") {
			if (i + 1 < len && str[i + 1] == '=') {
				s += str[++i];
			}
			else if (i + 1 < len && str[i + 1] == s[0]) {
				s += str[++i];
				if (i + 1 < len && str[i + 1] == '=') {
					s += str[++i];
				}
			}
		}
		w = { 4,s };
	}
	vt.push_back(w);
	++i;
}

/*��һ���ַ������дʷ�����*/
void analysis(string str) {
	//����һ�еķ������
	vector<Words> vt;
	int len = str.length();
	for (int i = 0; i < len; ) {
		char c = str[i];
		//��ascii���ַ�
		if (!isascii(c)) {
			handleNoAsciiSymble(str, i, len, vt);
		}
		//�ո�ֱ������
		else if (isblank(c)) {
			++i;
			continue;
		}
		//��ĸ��'_' �����������ַ���
		else if (isalpha(c) || c == '_') {
			handleKeywordOrIdentifier(str, i, len, vt);
		}
		//���� �������������ֳ���
		else if (isdigit(c)) {
			handleDigitConstant(str, i, len, vt);
		}
		//������ ����һ��������֮�����һ���ַ�
		else if (c == '\'') {
			handleCharacterConstant(str, i, len, vt);
		}
		//˫���� ����һ��˫����֮����԰��������ַ�
		else if (c == '\"') {
			handleStringConstant(str, i, len, vt);
		}
		//�����ַ�
		else {
			handleOtherSymbol(str, i, len, vt);
		}
	}
	result.push_back(vt);
}

/*��ӡ���*/
void printfResult() {
	int idx = 1;
	for (vector<Words> vt : result) {
		cout << "\n��" << idx << "��:" << endl;
		for (Words w : vt) {
			w.printf();
		}
		cout << endl;
		++idx;
	}
	cout << endl;
}

int main() {
	//��ʼ��
	init();
	while (true) {
		system("cls");
		//������ʾ
		cout << "-------�ʷ�����------\n" << "���ִ��������:\n"
			<< "********************************************************\n"
			<< " 1:������    2:��ʶ��    3:����    4:�����    5:�ָ���\n"
			<< " 6:�쳣�ֶ�  7:����ע��\n"
			<< "********************************************************\n"
			<< "��ѡ�Ĳ���:   1.����c���Գ���txt�ļ���ַ\n"
			<< "              2.�ֶ�����һ��c���Դ���\n"
			<< "              3.�˳�\n"
			<< "��ѡ�����: ";
		string s; 
		getline(cin, s);
		//txt�ļ���ȡģ�� (Ҫ�����result)
		if (s == "1") {
			result.clear();
			cout << "����c���Գ���txt�ļ���ַ��";
			string filePath;
			getline(cin, filePath);
			//�ļ���ȡ
			ifstream fin(filePath);
			string str;
			//ÿ�ζ�ȡһ�� (��ʵ����cin��ֻ࣬������Դ��ͬ)
			while (getline(fin, str)) {
				//cout << str;
				analysis(str);
			}
			printfResult();
		}
		//�ֶ�����ģ�� (һ��) (Ҫ�����result)
		else if (s == "2") {
			result.clear();
			cout << "����һ��c���Դ���:\n";
			string str;
			getline(cin, str);
			analysis(str);
			printfResult();
		}
		else if (s == "3") {
			cout << "�˳��ɹ�" << endl;
			break;
		}
		else {
			cout << "��Ч����" << endl;
		}
		system("pause");
	}
}