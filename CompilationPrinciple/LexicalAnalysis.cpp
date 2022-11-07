#include "iostream"
#include "string"
#include "cstring"
#include "algorithm"
#include "map"
#include "vector"
#include "fstream"
using namespace std;

//用于保证每段单词的类型和内容
struct Words {
	int type;
	string word;
	void printf() {
		cout << "(" << type << ", " << word << ")  ";
	}
};

//查找map，用于直接匹配保留字、运算符、分隔符
map<string, int> findMap;
//储存分析结果
vector<vector<Words>>  result;

/*初始化函数，生成查找map*/
void init() {
	//1. c语言保留字(注意main不是保留字)
	string keyword[] = { "if","else","while","do","int","float","double","return","const","void",
		"continue","break","char","signed","enum","long","switch","case","auto","unsigned" };
	for (string s : keyword) {
		//插入查找map中
		findMap[s] = 1;
	}
	//2. 标识符 匹配规则：<字母>|<下划线>(<字母>|<下滑线>|<数字>) (不能是关键字) -> 对应编号 2

	//3. 常数 包括数字常数(正数/小数)、字符常数(单引号中间一个字符:'a')、
	//        字符串(c语言提供了字符串，但没提供字符串类型，双引号中间字符串:"") -> 对应编号 3

	//4. 运算符 (多个字符组成的运算符实际处理时是先匹配到第一个字符，在看下一个是否可以组合)
	string operationSymbol[] = { ".","->","!","~","++","--","+","-","&","/","%","<<",">>",">","<","<=",
		">=","==","!=","^","|","&&","||","?",":","=","+=","-=","*=","/=","&=","%=","^=" ,"|=","<<=,",">>=" };
	for (string s : operationSymbol) {
		findMap[s] = 4;
	}

	//5. 分隔符
	string separator[] = { "(",")","[","]","{","}",",",";","#"};
	for (string s : separator) {
		findMap[s] = 5;
	}
	//6. 异常情况
}

/*处理非ascii码字符*/
void handleNoAsciiSymble(string& str, int& i, int len, vector<Words>& vt) {
	string s = "";
	while (i < len && (!isascii(str[i]))) {
		s += str[i++];
	}
	Words w = { 6,s };
	vt.push_back(w);
}

/*处理保留字或标识符*/
void handleKeywordOrIdentifier(string& str, int& i, int len, vector<Words>& vt) {
	string s = "";
	while (i < len && (isalpha(str[i]) || str[i] == '_' || isdigit(str[i]))) {
		s += str[i];
		++i;
	}
	if (findMap.find(s) != findMap.end()) {
		//保留字
		Words w = { 1,s };
		vt.push_back(w);
	}
	else {
		//标识符
		Words w = { 2,s };
		vt.push_back(w);
	}
}

/*处理数字常量*/
void handleDigitConstant(string& str, int& i, int len, vector<Words>& vt) {
	string s = "";
	//出现的小数点的数量
	int dotNum = 0;
	while (i < len && (isdigit(str[i]) || str[i] == '.')) {
		if (str[i] == '.') {
			++dotNum;
		}
		s += str[i++];
	}
	Words w;
	if (dotNum > 1) {
		//出现两个以上的小数点 -> 异常字段
		w = { 6,s };
	}
	else {
		w = { 3,s };
	}
	vt.push_back(w);
}

/*处理字符常量*/
void handleCharacterConstant(string& str, int& i, int len, vector<Words>& vt) {
	string s = "\'";
	++i;
	Words w;
	//如果不是'a'格式 -> 异常数据
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

/*处理字符串常量*/
void handleStringConstant(string& str, int& i, int len, vector<Words>& vt) {
	string s = "\"";
	++i;
	while (i < len && str[i] != '\"') {
		s += str[i++];
	}
	Words w;
	//结尾没有" -> 异常数据
	if (i >= len || str[i] != '\"') {
		w = { 6,s };
	}
	else {
		s += str[i++];
		w = { 3,s };
	}
	vt.push_back(w);
}

/*处理其他字符*/
void handleOtherSymbol(string& str, int& i, int len, vector<Words>& vt) {
	Words w;
	string s = "";
	s += str[i];
	map<string, int>::iterator item = findMap.find(s);
	//非法字符 -> 异常字段
	if (item == findMap.end()) {
		w = { 6,s };
	}
	//分隔符
	else if (item->second == 5) {
		w = { 5,s };
	}
	//单行注释的处理 -> 后面一整行都是注释
	// "/" -》 "//"
	else if (s == "/" && i + 1 < len && str[i + 1] == '/') {
		++i;
		while (i < len) {
			s += str[i++];
		}
		w = { 7,s };
	}
	//运算符 有些要往后读多个
	else {
		// "+"  -》  "++" / "+="
		if (s == "+") {
			if (i + 1 < len && str[i + 1] == '+' || str[i + 1] == '=') {
				s += str[++i];
			}
		}
		//"-"  -》  "->" / "--" / "-="
		else if (s == "-") {
			if (i + 1 < len && str[i + 1] == '>' || str[i + 1] == '-' || str[i + 1] == '=') {
				s += str[++i];
			}
		}
		//"*""/""%""^""!""="  -》 "*=""/=""%=""^=""!=""=="
		else if (s == "*" || s == "/" || s == "%" || s == "|" || s == "!" || s == "=") {
			if (i + 1 < len && str[i + 1] == '=') {
				s += str[++i];
			}
		}
		//"&""|"  -》  "&&""||" / "&=""|="
		else if (s == "&" || s == "|") {
			if (i + 1 < len && (str[i + 1] == s[0] || str[i + 1] == '=')) {
				s += str[++i];
			}
		}
		//">""<"  -》 ">=""<=" / ">>""<<"  -》 ">>=""<<="
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

/*对一行字符串进行词法分析*/
void analysis(string str) {
	//储存一行的分析结果
	vector<Words> vt;
	int len = str.length();
	for (int i = 0; i < len; ) {
		char c = str[i];
		//非ascii码字符
		if (!isascii(c)) {
			handleNoAsciiSymble(str, i, len, vt);
		}
		//空格直接跳过
		else if (isblank(c)) {
			++i;
			continue;
		}
		//字母或'_' 向后查找整个字符串
		else if (isalpha(c) || c == '_') {
			handleKeywordOrIdentifier(str, i, len, vt);
		}
		//数字 向后查找整个数字常数
		else if (isdigit(c)) {
			handleDigitConstant(str, i, len, vt);
		}
		//单引号 与下一个单引号之间包含一个字符
		else if (c == '\'') {
			handleCharacterConstant(str, i, len, vt);
		}
		//双引号 与下一个双引号之间可以包含任意字符
		else if (c == '\"') {
			handleStringConstant(str, i, len, vt);
		}
		//其他字符
		else {
			handleOtherSymbol(str, i, len, vt);
		}
	}
	result.push_back(vt);
}

/*打印结果*/
void printfResult() {
	int idx = 1;
	for (vector<Words> vt : result) {
		cout << "\n第" << idx << "行:" << endl;
		for (Words w : vt) {
			w.printf();
		}
		cout << endl;
		++idx;
	}
	cout << endl;
}

int main() {
	//初始化
	init();
	while (true) {
		system("cls");
		//界面提示
		cout << "-------词法分析------\n" << "数字代表的种类:\n"
			<< "********************************************************\n"
			<< " 1:保留字    2:标识符    3:常数    4:运算符    5:分隔符\n"
			<< " 6:异常字段  7:单行注释\n"
			<< "********************************************************\n"
			<< "可选的操作:   1.输入c语言程序txt文件地址\n"
			<< "              2.手动输入一行c语言代码\n"
			<< "              3.退出\n"
			<< "请选择操作: ";
		string s; 
		getline(cin, s);
		//txt文件读取模块 (要先清空result)
		if (s == "1") {
			result.clear();
			cout << "输入c语言程序txt文件地址：";
			string filePath;
			getline(cin, filePath);
			//文件读取
			ifstream fin(filePath);
			string str;
			//每次读取一行 (其实和用cin差不多，只是输入源不同)
			while (getline(fin, str)) {
				//cout << str;
				analysis(str);
			}
			printfResult();
		}
		//手动输入模块 (一行) (要先清空result)
		else if (s == "2") {
			result.clear();
			cout << "输入一行c语言代码:\n";
			string str;
			getline(cin, str);
			analysis(str);
			printfResult();
		}
		else if (s == "3") {
			cout << "退出成功" << endl;
			break;
		}
		else {
			cout << "无效输入" << endl;
		}
		system("pause");
	}
}