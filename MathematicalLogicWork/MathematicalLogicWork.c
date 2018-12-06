#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <wchar.h>
#include <wctype.h>
#include <math.h>

#pragma region MACRO


#define MAXLENTH 1000 //单行最长长度
#define MAXPARANUM 10 //函数最大参数数量
#define IDENTIFIER_LENGTH 10 //标识符最大长度
#define VARIABLE_NUMBER 10 //命题变元最大数量
#define FUNCTION_NUMBER 10 //函数最大数量
#define TRUTHTABLE_LENGTH 1024 //真值表最大长度
#define MAXSTACKDEPTH 10000

#pragma endregion

#pragma region GLOBAL


FILE *inputFile;
FILE *outputFile;
int layer = 0; //当前所处的语法树深度
int startIndex = 0; //getSymbol运行前index所处的位置
int index = 0; //当前词法分析到的位置
wchar_t currentSymbol[IDENTIFIER_LENGTH + 1]; //目前分析得到的标识符内容

struct function //函数列表
{
	wchar_t name[IDENTIFIER_LENGTH + 1];
	int paraNumber;
	wchar_t paras[MAXPARANUM];
	wchar_t truthTable[TRUTHTABLE_LENGTH];

	//计算函数时，先获取其参数的值，然后调用
	/*
	按照参数顺序计算值，然后查找真值表，返回值
	*/

}functionList[FUNCTION_NUMBER];
int funIndex = 0;

struct variable //命题变元列表
{
	wchar_t name[IDENTIFIER_LENGTH + 1];
	int truthValue;
	//计算公式时，首先赋值，然后计算值，计算值时先查表
}variableList[VARIABLE_NUMBER];
int varIndex = 0;

struct runtimeFrame
{
	int layer;
	int type;//1 变元 2 第一类 3 自定义
	int value;
	int paraNumber;
	wchar_t symbol[IDENTIFIER_LENGTH + 1];
}runtimeStack[MAXSTACKDEPTH],tempStack[MAXSTACKDEPTH];
int stackIndex = 0;
int tempStackIndex = 0;

const enum symbol //词法符号
{
	LBRACE = 0, RBRACE, COMMA, SHARP,
	TRUE, FALSE, AND, OR, NOT,
	EXCLUSIVEOR, IMPLICATION, EQUIVALENCE,
	IDENTIFIER = 12, DIGITSTR
};

const wchar_t *symbolName[] =
		{
				L"LBRACE", L"RBRACE", L"COMMA", L"SHARP",
				L"TRUE", L"FALSE", L"AND", L"OR", L"NOT",
				L"EXCLUSIVEOR", L"IMPLICATION", L"EQUIVALENCE",
				L"IDENTIFIER", L"DIGITSTR"
		};

const wchar_t symbolSet[] =
		{
				L'(', L')', L',', L'#',
				L'1', L'0', L'∧', L'∨', L'¬',
				L'⊕', L'→', L'↔'
		};

#pragma endregion


#pragma region FUNCTION

void jumpSpace(wchar_t *expression);
int getSymbol(wchar_t *expression);


void parseExpression(wchar_t *expression);
void parseConj(wchar_t *expression);
void parseLevelFiveItem(wchar_t *expression);
void parseLevelFourItem(wchar_t *expression);
void parseLevelThreeItem(wchar_t *expression);
void parseLevelTwoItem(wchar_t *expression);
void parseLevelOneItem(wchar_t *expression);
void parseFactor(wchar_t *expression);

#pragma endregion

#pragma region SYMBOL

int searchVarList(wchar_t *name);
int searchFunList(wchar_t *name);

#pragma endregion

#pragma region RUNTIMESTACK

void pushConjOne(int symbolIndex, int layer, int paraNumber, int type);
void getTruthValue(wchar_t * name, int type, int runtimeIndex);
void getExpTruthValue();
void moveStack(int runtimeIndex);

#pragma endregion


#pragma region TODO
/*
TODO:
生成运算栈
计算运算栈时，对每一个逻辑联结词，包括自定义联结词
按照layer从大到小，按照出现的先后顺序从先到后计算运算符
TODO:
关于运行栈
第一类联结词中 0和1 可以直接计算值，考虑针对不同的联结词添加不同的函数
*/

#pragma endregion



int main(int argc, char *argv[])
{
	errno_t err;
	wchar_t expression[MAXLENTH];

	int _symbol;

	if (argc == 3)
	{
		printf("%s %s\n", argv[1], argv[2]);
		err = fopen_s(&inputFile, argv[1], "r,ccs=UNICODE");
		if (err!=0)
		{
			printf("NO INPUT FILE:%s\n",argv[1]);
			return 1;
		}
		err = fopen_s(&outputFile, argv[2], "w,ccs=UNICODE");
		if (err!=0)
		{
			printf("NO OUTPUT FILE:%s\n", argv[2]);
			return 1;
		}
	}
	else
	{
		printf("PARAMETER NUMBER ERROR:%d", argc - 1);
		return 1;
	}


	while (!feof(inputFile))
	{
		fgetws(expression, MAXLENTH - 1, inputFile);
		if (expression[wcslen(expression) - 1] == L'\n')
		{
			expression[wcslen(expression) - 1] = L'\0';
		}


		parseExpression(expression);
		wprintf(L"%s\n", expression);
		fwprintf(outputFile,L"%s\n",expression);

		getExpTruthValue();

		/*
		funIndex--;
		wprintf(L"Print Function Table:\n");
		while (funIndex>=0)
		{
			wprintf(L"Num.%d function : %s\n",funIndex,functionList[funIndex].name);
			wprintf(L"Number of parameters : %d\n",functionList[funIndex].paraNumber);
			wprintf(L"Truth Table : %s\n",functionList[funIndex].truthTable);
			funIndex--;
		}
		varIndex--;
		wprintf(L"Print Variable Table:\n");
		while (varIndex >= 0)
		{
			wprintf(L"Num.%d variable : %s\n", varIndex, variableList[varIndex].name);
			varIndex--;
		}
		stackIndex--;
		wprintf(L"Print Runtime Stack:\n");
		while (stackIndex >= 0)
		{
			wprintf(L"Num.%d symbol : %s at layer %d\n", stackIndex, runtimeStack[stackIndex].symbol, runtimeStack[stackIndex].layer);
			stackIndex--;
		}
		*/



		//归零四大天王
		wmemset(expression, 0, MAXLENTH-1);
		index = 0;
		funIndex = 0;
		varIndex = 0;
		stackIndex = 0;
	}
	fclose(inputFile);
	fclose(outputFile);


	getchar();
	return 0;
}

void jumpSpace(wchar_t *expression)
{
	while (expression[index] == L' ')
	{
		index++;
	}
}
void jumpBackward()
{
	wmemset(currentSymbol, 0, IDENTIFIER_LENGTH + 1);
	index = startIndex;
}

int getSymbol(wchar_t * expression)
{
	int _symbolIndex = 0;
	wmemset(currentSymbol, 0, IDENTIFIER_LENGTH + 1);
	startIndex = index;

	jumpSpace(expression);
	//printf("currentdex:%d sizeofexpression:%d ",index, wcslen(expression));

	if (index >= (int)wcslen(expression))
	{
		return -2;
	}

	switch (expression[index])
	{
		case L'(':
			index++;
			return LBRACE;
		case L')':
			index++;
			return RBRACE;
		case L',':
			index++;
			return COMMA;
		case L'#':
			index++;
			return SHARP;
		case L'1':
			currentSymbol[_symbolIndex++] = expression[index++];
			currentSymbol[_symbolIndex] = L'\0';
			return TRUE;
		case L'0':
			currentSymbol[_symbolIndex++] = expression[index++];
			currentSymbol[_symbolIndex] = L'\0';
			return FALSE;
		case L'∧':
			index++;
			return AND;
		case L'∨':
			index++;
			return OR;
		case L'¬':
			index++;
			return NOT;
		case L'⊕':
			index++;
			return EXCLUSIVEOR;
		case L'→':
			index++;
			return IMPLICATION;
		case L'↔':
			index++;
			return EQUIVALENCE;
		default:
			if (iswalpha(expression[index]))
			{
				while ((iswalpha(expression[index])||iswdigit(expression[index])))
				{
					currentSymbol[_symbolIndex++] = expression[index++];
				}
				currentSymbol[_symbolIndex] = L'\0';
				return IDENTIFIER;
			}
			else if (iswdigit(expression[index]))
			{
				while (iswdigit(expression[index]))
				{
					currentSymbol[_symbolIndex++] = expression[index++];
				}
				currentSymbol[_symbolIndex] = L'\0';
				return DIGITSTR;
			}
			return -1;
	}
}

void parseExpression(wchar_t * expression)
{
	layer++;
	//printf("parseExpression : Current in layer : %d\n", layer);
	if (getSymbol(expression)==SHARP)
	{
		jumpBackward();
		parseConj(expression);
	}
	else
	{
		jumpBackward();
		parseLevelFiveItem(expression);
		parseConj(expression);
	}
	//printf("parseExpression : Current out layer : %d\n", layer);
	layer--;
}

void parseConj(wchar_t * expression)
{
	layer++;
	//printf("parseConj : Current in layer : %d\n", layer);
	int _truthTableIndex = 0;
	int _searchResult = -1;
	int _sym = -1;

	if (getSymbol(expression) == SHARP)
	{
		while (getSymbol(expression) != -2)//读取所有的联结词定义直到达到文件末尾
		{
			jumpBackward();
			if (getSymbol(expression) == IDENTIFIER)
			{
				_searchResult = searchFunList(currentSymbol);
				if (_searchResult == -1)//新建一个联结词
				{
					wcsncpy(functionList[funIndex].name, currentSymbol, IDENTIFIER_LENGTH + 1);
					_sym = getSymbol(expression);
					if (_sym == DIGITSTR || _sym == TRUE || _sym == FALSE)
					{
						functionList[funIndex].paraNumber = _wtoi(currentSymbol);
						while (getSymbol(expression) != IDENTIFIER)
						{
							jumpBackward();
							if (getSymbol(expression) == -2){break;}
							else { jumpBackward(); }

							if (getSymbol(expression) == TRUE)
							{
								//putchar('1');
								functionList[funIndex].truthTable[_truthTableIndex++] = L'1';
								continue;
							}
							else
							{
								jumpBackward();
								if (getSymbol(expression) == FALSE)
								{
									//putchar('0');
									functionList[funIndex].truthTable[_truthTableIndex++] = L'0';
								}
								continue;
							}
						}
						//putchar('\n');
						jumpBackward();
					}
					functionList[funIndex].truthTable[_truthTableIndex] = L'\0';
					funIndex++;
				}
				else//检查是否一致并补全真值表
				{
					_sym = getSymbol(expression);
					if (_sym == DIGITSTR || _sym == TRUE || _sym == FALSE)
					{
						if (functionList[_searchResult].paraNumber == _wtoi(currentSymbol))
						{
							while (getSymbol(expression) != IDENTIFIER)
							{
								jumpBackward();
								if (getSymbol(expression) == -2){break;}
								else{ jumpBackward(); }

								if (getSymbol(expression) == TRUE)
								{
									//putchar('1');
									functionList[_searchResult].truthTable[_truthTableIndex++] = L'1';
									continue;
								}
								else
								{
									jumpBackward();
									if (getSymbol(expression) == FALSE)
									{
										//putchar('0');
										functionList[_searchResult].truthTable[_truthTableIndex++] = L'0';
									}
									continue;
								}
							}
							//putchar('\n');
							jumpBackward();
						}
						else{/*这里要报错*/}
					}
					functionList[_searchResult].truthTable[_truthTableIndex] = L'\0';
				}
				_truthTableIndex = 0;
				_searchResult = -1;
			}
		}
	}
	else
	{
		jumpBackward();
	}
	//printf("parseConj : Current out layer : %d\n", layer);
	layer--;
}

void parseLevelFiveItem(wchar_t * expression)
{
	layer++;
	//printf("parseLevelFiveItem : Current in layer : %d\n", layer);
	parseLevelFourItem(expression);
	while (getSymbol(expression) == EQUIVALENCE)
	{
		parseLevelFourItem(expression);
		//printf("current operation : EQUIVALENCE at layer %d\n",layer);
		//第一类联结词压栈
		pushConjOne(EQUIVALENCE, layer, 2,2);
	}
	jumpBackward();
	//printf("parseLevelFiveItem : Current out layer : %d\n", layer);
	layer--;
}

void parseLevelFourItem(wchar_t *expression)
{
	layer++;
	//printf("parseLevelFourItem : Current in layer : %d\n", layer);
	parseLevelThreeItem(expression);
	while (getSymbol(expression) == IMPLICATION)
	{
		parseLevelThreeItem(expression);
		//printf("current operation : IMPLICATION at layer %d\n", layer);
		//第一类联结词压栈
		pushConjOne(IMPLICATION, layer, 2,2);
	}
	jumpBackward();
	//printf("parseLevelFourItem : Current out layer : %d\n", layer);
	layer--;
}

void parseLevelThreeItem(wchar_t *expression)
{
	layer++;
	//printf("parseLevelThreeItem : Current in layer : %d\n", layer);
	parseLevelTwoItem(expression);
	while (getSymbol(expression) == EXCLUSIVEOR)
	{
		parseLevelTwoItem(expression);
		//printf("current operation : EXCLUSIVEOR at layer %d\n", layer);
		//第一类联结词压栈
		pushConjOne(EXCLUSIVEOR, layer, 2,2);
	}
	jumpBackward();
	//printf("parseLevelThreeItem : Current out layer : %d\n", layer);
	layer--;
}

void parseLevelTwoItem(wchar_t *expression)
{
	layer++;
	//printf("parseLevelTwoItem : Current in layer : %d\n", layer);
	parseLevelOneItem(expression);
	while (getSymbol(expression) == OR)
	{
		parseLevelOneItem(expression);
		//printf("current operation : OR at layer %d\n", layer);
		//第一类联结词压栈
		pushConjOne(OR, layer, 2,2);
	}
	jumpBackward();
	//printf("parseLevelTwoItem : Current out layer : %d\n", layer);
	layer--;
}

void parseLevelOneItem(wchar_t *expression)
{
	layer++;
	//printf("parseLevelOneItem : Current in layer : %d\n", layer);
	parseFactor(expression);
	while (getSymbol(expression)==AND)
	{
		parseFactor(expression);
		//printf("current operation : AND at layer %d\n", layer);
		//第一类联结词压栈
		pushConjOne(AND, layer, 2,2);
	}
	jumpBackward();
	//printf("parseLevelOneItem : Current out layer : %d\n", layer);
	layer--;
}

void parseFactor(wchar_t *expression)
{
	wchar_t tempIdentifier[IDENTIFIER_LENGTH + 1];
	int _paraNumber = 0;
	int _tempFunIndex = 0;
	int _searchFunResult = -1;

	layer++;
	//printf("parseFactor : Current in layer : %d\n", layer);

	switch (getSymbol(expression))
	{
		case NOT:
			parseFactor(expression);
			//printf("current operation : NOT at layer %d\n", layer);
			//第一类联结词压栈
			pushConjOne(NOT, layer, 1,2);
			break;
		case LBRACE:
			parseLevelFiveItem(expression);
			if (getSymbol(expression) == RBRACE){}
			break;
		case TRUE:
			//第一类联结词压栈
			pushConjOne(TRUE, layer, 0,2);
			break;
		case FALSE:
			//第一类联结词压栈
			pushConjOne(FALSE, layer, 0,2);
			break;
		case IDENTIFIER:
			wcsncpy(tempIdentifier,currentSymbol, IDENTIFIER_LENGTH + 1);
			if (getSymbol(expression) == LBRACE)//自定义联结词
			{//TODO
				_searchFunResult = searchFunList(tempIdentifier);
				if (_searchFunResult == -1)//当符号表中不存在时，则新建一个联结词
				{
					wcsncpy(functionList[funIndex].name, tempIdentifier, IDENTIFIER_LENGTH + 1);
					if (getSymbol(expression) == RBRACE)//该联结词参数数为0
					{
						functionList[funIndex].paraNumber = _paraNumber;
						funIndex++;
						//自定义联结词压栈
						wcsncpy(runtimeStack[stackIndex].symbol, tempIdentifier, IDENTIFIER_LENGTH + 1);
						runtimeStack[stackIndex].layer = layer;
						runtimeStack[stackIndex].type = 3;
						runtimeStack[stackIndex].paraNumber = _paraNumber;
						stackIndex++;

						break;
					}
					else //该联结词参数不为零
					{
						//预读了，退一位
						jumpBackward();
						//如果参数是函数，那么需要给自己占位，需要修改funIndex，等到运行完后再返回到原始位置
						_tempFunIndex = funIndex;

						funIndex++;
						parseLevelFiveItem(expression);
						_paraNumber++;

						while (getSymbol(expression) == COMMA)
						{
							//funIndex++;
							parseLevelFiveItem(expression);
							_paraNumber++;
						}
						//退出时应该预读了一个 RBRACE，退一位
						jumpBackward();
						if (getSymbol(expression) == RBRACE)
						{
							//wprintf(L"current operation : %s at layer %d\n", tempIdentifier, layer);
							functionList[_tempFunIndex].paraNumber = _paraNumber;
							//自定义联结词压栈
							wcsncpy(runtimeStack[stackIndex].symbol, tempIdentifier, IDENTIFIER_LENGTH + 1);
							runtimeStack[stackIndex].layer = layer;
							runtimeStack[stackIndex].type = 3;
							runtimeStack[stackIndex].paraNumber = _paraNumber;
							stackIndex++;

							break;
						}
					}
				}
				else//当符号表中存在时，检查与之前的是否一致
				{
					if (getSymbol(expression) == RBRACE)
					{
						if (functionList[_searchFunResult].paraNumber == 0)
						{
							//自定义联结词压栈
							wcsncpy(runtimeStack[stackIndex].symbol, tempIdentifier, IDENTIFIER_LENGTH + 1);
							runtimeStack[stackIndex].layer = layer;
							runtimeStack[stackIndex].type = 3;
							runtimeStack[stackIndex].paraNumber = _paraNumber;
							stackIndex++;
							break;
						}
						else {/*报错*/break; }
					}
					else
					{
						//预读了，退一位
						jumpBackward();
						//此时不需要给自己占位
						parseLevelFiveItem(expression);
						_paraNumber++;

						while (getSymbol(expression) == COMMA)
						{
							parseLevelFiveItem(expression);
							_paraNumber++;
						}
						//退出时应该预读了一个 RBRACE，退一位
						jumpBackward();
						if (getSymbol(expression) == RBRACE)
						{
							//wprintf(L"current operation : %s at layer %d\n", tempIdentifier, layer);
							functionList[_searchFunResult].paraNumber = _paraNumber;

							//自定义联结词压栈
							wcsncpy(runtimeStack[stackIndex].symbol, tempIdentifier, IDENTIFIER_LENGTH + 1);
							runtimeStack[stackIndex].layer = layer;
							runtimeStack[stackIndex].type = 3;
							runtimeStack[stackIndex].paraNumber = _paraNumber;
							stackIndex++;
							break;
						}
					}
				}
			}
			else//命题变元
			{
				//预读了，所以要退一位
				jumpBackward();
				_searchFunResult = searchVarList(tempIdentifier);
				if (_searchFunResult == -1)//首先查找是否定义了这个变元
				{
					//新建变元
					wcsncpy(variableList[varIndex].name, tempIdentifier, IDENTIFIER_LENGTH + 1);
					varIndex++;
					//压栈
					wcsncpy(runtimeStack[stackIndex].symbol, variableList[varIndex-1].name, IDENTIFIER_LENGTH + 1);
					runtimeStack[stackIndex].layer = layer;
					runtimeStack[stackIndex].type = 1;
					stackIndex++;
				}
				else//找到了就直接压栈
				{
					wcsncpy(runtimeStack[stackIndex].symbol, variableList[_searchFunResult].name, IDENTIFIER_LENGTH + 1);
					runtimeStack[stackIndex].layer = layer;
					runtimeStack[stackIndex].type = 1;
					stackIndex++;
				}
				break;
			}
			break;
		default:
			jumpSpace(expression);
			break;
	}
	//printf("parseFactor : Current out layer : %d\n", layer);
	layer--;
}

int searchVarList(wchar_t *name)
{
	int i = 0;
	for (; i < varIndex; i++)
	{
		if (wcsncmp(variableList[i].name, name, wcslen(name)) == 0)
		{
			return i;
		}
	}
	return -1;
}

int searchFunList(wchar_t * name)
{
	int i = 0;
	for (; i < funIndex; i++)
	{
		if (wcsncmp(functionList[i].name, name, wcslen(name)) == 0)
		{
			return i;
		}
	}
	return -1;
}

void pushConjOne(int symbolIndex, int layer, int paraNumber, int type)
{
	//第一类联结词压栈
	runtimeStack[stackIndex].symbol[0] = symbolSet[symbolIndex];
	runtimeStack[stackIndex].symbol[1] = L'\0';
	runtimeStack[stackIndex].layer = layer;
	runtimeStack[stackIndex].type = type;
	runtimeStack[stackIndex].paraNumber = paraNumber;
	if (symbolIndex == 4){runtimeStack[stackIndex].value = 1;}
	else if (symbolIndex == 5){runtimeStack[stackIndex].value = 0;}
	else{runtimeStack[stackIndex].value = -1;}
	stackIndex++;
}

void getTruthValue(wchar_t * name,int type,int runtimeIndex)
{
	//getTruthValue前要进行赋值操作，假设调用该函数时，联结词所需的变量已在对应位置
	int _trueValueIndex = 0;
	int _searchResult = -1;
	int i = 0;
	runtimeStack[runtimeIndex].layer = 0;
	switch (type)
	{
		case 1:
			runtimeStack[runtimeIndex].value = variableList[searchVarList(runtimeStack[runtimeIndex].symbol)].truthValue;
			return;
		case 2://第一类联结词
			switch (name[0])
			{
				case L'0':
					return;
				case L'1':
					return;
				case L'∧':
					runtimeStack[runtimeIndex].value =  runtimeStack[runtimeIndex - 2].value & runtimeStack[runtimeIndex - 1].value;
					moveStack(runtimeIndex);
					return;
				case L'∨':
					runtimeStack[runtimeIndex].value = runtimeStack[runtimeIndex - 2].value | runtimeStack[runtimeIndex - 1].value;
					moveStack(runtimeIndex);
					return;
				case L'¬':
					runtimeStack[runtimeIndex].value = (runtimeStack[runtimeIndex - 1].value==0)?1:0;
					moveStack(runtimeIndex);
					return;
				case L'⊕':
					runtimeStack[runtimeIndex].value =  (runtimeStack[runtimeIndex - 2].value == runtimeStack[runtimeIndex - 1].value)?0:1;
					moveStack(runtimeIndex);
					return;
				case L'→':
					if (runtimeStack[runtimeIndex - 2].value == 0 ||
						(runtimeStack[runtimeIndex - 2].value == 1 && runtimeStack[runtimeIndex - 1].value == 1))
					{
						runtimeStack[runtimeIndex].value = 1;
						moveStack(runtimeIndex);
						return;
					}
					else
					{
						runtimeStack[runtimeIndex].value = 0;
						moveStack(runtimeIndex);
						return;
					}

				case L'↔':
					if (runtimeStack[runtimeIndex - 2].value == runtimeStack[runtimeIndex - 1].value)
					{
						runtimeStack[runtimeIndex].value = 1;
						moveStack(runtimeIndex);
						return;
					}
					else
					{
						runtimeStack[runtimeIndex].value = 0;
						moveStack(runtimeIndex);
						return;
					}
			}
		case 3://第二类联结词
			_searchResult = searchFunList(name);
			for (i = runtimeIndex - functionList[_searchResult].paraNumber;i<runtimeIndex;i++)
			{
				_trueValueIndex *= 2;
				_trueValueIndex += runtimeStack[i].value;
			}
			runtimeStack[runtimeIndex].value = (functionList[_searchResult].truthTable[_trueValueIndex]-L'0');
			moveStack(runtimeIndex);
			return;
	}
}

void getExpTruthValue()
{
	int i = 0;
	int j = 0;
	int _deepest = -1;
	int _repeatTimes = pow(2, varIndex);

	//计算之前，需要复制运行栈
	tempStackIndex = stackIndex;
	for (j = 0; i < stackIndex; i++)
	{
		tempStack[i] = runtimeStack[i];
	}

	for (j = 0; j < varIndex; j++)//对命题变元赋值
	{
		wprintf(L"%s\t", variableList[j].name);
		fwprintf(outputFile,L"%s\t",variableList[j].name);
	}
	printf("TruthValue\n");
	fputws(L"TruthValue\n", outputFile);

	for (i = 0; i < _repeatTimes;i++)//对所有取值进行赋值
	{
		for (j = 0; j < varIndex; j++)//对命题变元赋值
		{
			variableList[j].truthValue = ((1 << (varIndex - 1 - j))& i) >> (varIndex - 1 - j);
			printf("%d\t", variableList[j].truthValue);
			fwprintf(outputFile,L"%d\t", variableList[j].truthValue);

		}

		for (j = 0; j < stackIndex; j++)//将赋值带入运行栈
		{
			if (runtimeStack[j].type == 1)
			{
				getTruthValue(runtimeStack[j].symbol, 1, j);
			}
		}
		while (stackIndex > 1)
		{
			for (j = 0; j < stackIndex; j++)//找到第一个最深
			{
				if (runtimeStack[j].layer > _deepest && runtimeStack[j].type != 1)
				{
					_deepest = runtimeStack[j].layer;
				}
			}
			for (j = 0; j < stackIndex; j++)//执行完所有同深度的函数，修改getTruthValue可能会好些
			{
				if (runtimeStack[j].layer == _deepest && runtimeStack[j].type != 1)//只要不是变元就执行
				{//bug在这个j
					getTruthValue(runtimeStack[j].symbol, runtimeStack[j].type, j);
					j = 0;
				}
			}
			_deepest = -1;
		}
		printf("%d\n",runtimeStack[stackIndex-1].value);
		fwprintf(outputFile,L"%d\n", runtimeStack[stackIndex - 1].value);
		//把运行栈复制回去
		stackIndex = tempStackIndex;
		for (j = 0; j < tempStackIndex; j++)
		{
			runtimeStack[j] = tempStack[j];
		}
	}
}

void moveStack(int runtimeIndex)
{
	int i;
	int len = runtimeStack[runtimeIndex].paraNumber;
	for (i = runtimeIndex; i < stackIndex; i++)
	{
		runtimeStack[i - len] = runtimeStack[i];
	}
	stackIndex -= len;
}