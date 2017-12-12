#include<stdio.h>
#include<windows.h>

//Errors
const char OOM = 1;//Out Of Memory
const char BAD_INPUT = 2;

inline bool compareArrays(const char *first, const char *second, int length)
{
	for (int ii = 0; ii < length; ++ii)
	{
		if (first[ii] != second[ii])
			return false;
	}

	return true;
}

inline bool equalString(const char *canonical, int canLength, const char *variant, int varLength)
{
	if (canLength != varLength)
		return false;

	for (int ii = 0; ii < canLength; ++ii)
	{
		if (canonical[ii] != variant[ii])
			return false;
	}

	return true;
}

int stringDiff(const char *canonical, int canLength, const char *variant, int varLength)
{
	int minLength = canLength>varLength ? canLength : varLength;
	int ii = 0;

	for (ii = 0; ii < minLength; ++ii)
	{
		if (canonical[ii] != variant[ii])
			return ii;
	}

	return ii;
}

bool oldVariantAddition(const char *canonical, int canLength, const char *variant, int varLength)
{
	if (varLength < canLength)
		return false;

	if (equalString(canonical, canLength, variant, varLength))
	{
		return true;
	}
	else
	{
		int diffIndex = stringDiff(canonical, canLength, variant, varLength);

		if (2 * diffIndex > varLength)
			diffIndex = varLength / 2; //Due to integer math, this equals the floor()

		char* newBuffer = (char*)malloc(varLength*sizeof(char));

		//Progressively move length UP to diffIndex
		for (int index = 0; index < diffIndex; ++index)
		{
			int subIndex = diffIndex;

			//Move subIndex DOWN, shortening towards index
			for (; index < subIndex; --subIndex)
			{
				int innerLength = subIndex - index;
				if (equalString(variant + index, innerLength, variant + subIndex, innerLength))
				{
					//Selectively copy elements from variant to newBuffer (forming the new variant to be tested)
					int newIndex = 0;
					for (int ii = 0; ii < varLength; ++ii, ++newIndex)
					{
						if (ii >= index && ii < subIndex)
							ii = subIndex;

						newBuffer[newIndex] = variant[ii];
					}

					if (oldVariantAddition(canonical, canLength, newBuffer, varLength - innerLength))//Perform new variant addition and return result if true
					{
						free(newBuffer);
						return true;
					}
				}
			}
		}

		free(newBuffer);
		return false;
	}
}

bool variantAdditionResInner(const char *canonical, int canLength, char *variant, int varLength)
{
	if (varLength > canLength)
	{
		int diffIndex = stringDiff(canonical, canLength, variant, varLength);

		if (2 * diffIndex > varLength)
			diffIndex = varLength / 2; //Due to integer math, this equals the floor()

		//Assumes that the buffer passed into variant has enough length to cover #varLength - #canLength additional recursions of this algorithm
		char* newVarient = variant + varLength;

		//Progressively move length UP to diffIndex
		for (int index = 0; index < diffIndex; ++index)
		{
			int subIndex = diffIndex;

			//Move subIndex DOWN, shortening towards index
			for (; index < subIndex; --subIndex)
			{
				int innerLength = subIndex - index;
				if (equalString(variant + index, innerLength, variant + subIndex, innerLength))
				{
					//Selectively copy elements from variant to newVarient, skipping those between index and subIndex
					int ii = 0;
					for (; ii < index; ++ii)
					{
						newVarient[ii] = variant[ii];
					}

					int newIndex = ii;
					ii = subIndex;

					for (; ii < varLength; ++ii, ++newIndex)
					{
						newVarient[newIndex] = variant[ii];
					}

					if (variantAdditionResInner(canonical, canLength, newVarient, newIndex))//Perform new variant addition and return result if true
					{
						return true;
					}
				}
			}
		}
		return false;
	}
	else
	{
		return equalString(canonical, canLength, variant, varLength);
	}
}


bool variantAdditionInner(const char *canonical, int canLength, char *variant, int varLength)
{
	if (varLength > canLength)
	{
		int diffIndex = diffIndex = varLength / 2; //Due to integer math, this equals the floor()
			
		//Assumes that the buffer passed into variant has enough length to cover #varLength - #canLength additional recursions of this algorithm
		char* newVarient = variant + varLength;

		//Progressively move length UP to diffIndex
		for (int index = 0; index < diffIndex; ++index)
		{
			int subIndex = diffIndex;

			//Move subIndex DOWN, shortening towards index
			for (; index < subIndex; --subIndex)
			{
				int innerLength = subIndex - index;
				if (equalString(variant + index, innerLength, variant + subIndex, innerLength))
				{
					//Selectively copy elements from variant to newVarient, skipping those between index and subIndex
					int ii = 0;
					for (; ii < index; ++ii)
					{
						newVarient[ii] = variant[ii];
					}

					int newIndex = ii;
					ii = subIndex;

					for (; ii < varLength; ++ii, ++newIndex)
					{
						newVarient[newIndex] = variant[ii];
					}

					if (variantAdditionInner(canonical, canLength, newVarient, newIndex))//Perform new variant addition and return result if true
					{
						return true;
					}
				}

				//NOTE: perform this computation selectively given the case that diffIndex*2 != varLength

				//perform a reverse cpmparison
				int reverseIndex = varLength - (2 * subIndex) + index;
				int reverseSubIndex = varLength - subIndex;
				if (equalString(variant + reverseIndex, innerLength, variant + reverseSubIndex, innerLength))
				{
					//Selectively copy elements from variant to newVarient, skipping those between index and subIndex
					int ii = 0;
					for (; ii < reverseIndex; ++ii)
					{
						newVarient[ii] = variant[ii];
					}

					int newIndex = ii;
					ii = reverseSubIndex;

					for (; ii < varLength; ++ii, ++newIndex)
					{
						newVarient[newIndex] = variant[ii];
					}

					if (variantAdditionInner(canonical, canLength, newVarient, newIndex))//Perform new variant addition and return result if true
					{
						return true;
					}
				}
			}
		}



		return false;
	}
	else
	{
		return equalString(canonical, canLength, variant, varLength);
	}
}

bool variantAddition(const char *canonical, int canLength, const char *variant, int varLength, char *error)
{
	if (canLength > varLength)
	{
		return false; //Prevents varlLength from not being greater than canLength, causing a call to malloc that would be equal or less than 0
	}
	else
	{
		//Allocate (n^2 + n)/2 - (m^2 + m)/2 bytes, where n == varLength and m == (canLength - 1), which should be the maximum needed size given varLength decreases by at least 1 each recursion
		int minorLength = canLength - 1;
		char *newBuffer = (char*)malloc((varLength*varLength + varLength - minorLength*minorLength - minorLength) * sizeof(char) / 2);

		if (newBuffer)
		{
			for (int ii = 0; ii < varLength; ++ii)
			{
				newBuffer[ii] = variant[ii];
			}

			bool val = variantAdditionInner(canonical, canLength, newBuffer, varLength);

			free(newBuffer);

			return val;
		}
		else
		{
			//Won't handle errors correctly
			*error |= OOM;
			return false;
		}
	}
}

bool strVariantAddition(const char *canonical, const char *variant, char *error)
{
	return variantAddition(canonical, strlen(canonical), variant, strlen(variant), error);
}


bool pureStrictBinaryVariantAddition(const char *canonical, int canLength, const char *variant, int varLength)
{
	if (varLength > canLength)
	{
		int innerLength = varLength / 2;

		if (varLength % 2 == 0
			&& compareArrays(variant, variant + innerLength, innerLength)
			&& pureStrictBinaryVariantAddition(canonical, canLength, variant, innerLength)) /*Half point Sub-strings exist and match*/
		{
			return true;
		}
		else if (canLength > 1)/*Half-point recursion was not possible or didn't return a positive result*/
		{
			//If possible (i.e. canLength > 1), perform split, running through variant splits from ([0,1] [1,varLength]) to ([0,varLength-1] [varLength-1,varLength]), and -for each variant split- run through all possible canonical splits and recurse through
			for (int varSplit = 1; varSplit < varLength; ++varSplit)
			{
				for (int canSplit = 0; canSplit < canLength; ++canSplit)
				{
					if (pureStrictBinaryVariantAddition(canonical, canSplit, variant, varSplit) && pureStrictBinaryVariantAddition(canonical + canSplit, canLength - canSplit, variant + varSplit, varLength - varSplit))
						return true;
				}
			}
		}

		return false;
	}
	else
	{
		return equalString(canonical, canLength, variant, varLength);
	}
}


//Variant expects a buffer containing varLength^2 bytes of memory to be passed to it
bool strictBinaryVariantAdditionInner(const char *canonical, int canLength, const char *variant, int varLength)
{
	if (varLength > canLength)
	{
		if (canonical[0] != variant[0] || canonical[canLength - 1] != variant[varLength - 1])
		{
			return false;
		}

		int innerLength = varLength / 2;

		if (varLength % 2 == 0
			&& compareArrays(variant, variant + innerLength, innerLength)
			&& strictBinaryVariantAdditionInner(canonical, canLength, variant, innerLength)) /*Half point Sub-strings exist and match*/
		{
			//Recursivly perform variant addition on one of the substrings
			return true;
			//Return true if this works
			//otherwise...
		}
		else if (canLength > 1)/*Half-point recursion was not possible or didn't return a positive result*/
		{
			//If possible (i.e. canLength > 1), perform split, running through variant splits from ([0,1] [1,varLength]) to ([0,varLength-1] [varLength-1,varLength]), and -for each variant split- run through all possible canonical splits and recurse through
			for (int varSplit = 1; varSplit < varLength; ++varSplit)
			{
				//int upperSplit = canLength < varSplit + 1 ? canLength : varSplit + 1;
				int upperSplit = min(canLength, varSplit + 1);
				//int current = canLength - 1 < varLength - varSplit ? canLength - 1: varLength - varSplit;
				int current = min(canLength - 1, varLength - varSplit);
				for (int canSplit = canLength - current; canSplit < upperSplit; ++canSplit)
				{
					/*if (strictBinaryVariantAddition(canonical, canSplit, variant, varSplit) && strictBinaryVariantAddition(canonical + canSplit, canLength - canSplit, variant + varSplit, varLength - varSplit))
						return true;*/

					if (canSplit <= canLength / 2)
					{
						if (strictBinaryVariantAdditionInner(canonical, canSplit, variant, varSplit) && strictBinaryVariantAdditionInner(canonical + canSplit, canLength - canSplit, variant + varSplit, varLength - varSplit))
						{
							return true;
						}
					}
					else
					{
						if (strictBinaryVariantAdditionInner(canonical + canSplit, canLength - canSplit, variant + varSplit, varLength - varSplit) && strictBinaryVariantAdditionInner(canonical, canSplit, variant, varSplit))
						{
							return true;
						}
					}
				}
			}
		}

		return false;
	}
	else
	{
		return equalString(canonical, canLength, variant, varLength);
	}
}

bool strictBinaryVariantAddition(const char *canonical, int canLength, const char *variant, int varLength)
{
	if (varLength > canLength)
	{
		if (canonical[0] != variant[0] || canonical[canLength - 1] != variant[varLength - 1])
		{
			return false;
		}
		else
		{
			strictBinaryVariantAdditionInner(canonical, canLength, variant,varLength);
		}
	}
	else
	{
		return equalString(canonical, canLength, variant, varLength);
	}
}

bool strSBVariantAddition(const char *canonical, const char *variant)
{
	return strictBinaryVariantAddition(canonical, strlen(canonical), variant, strlen(variant));
}


struct additionData
{
	size_t tableSize;
	size_t canonicalIndex;
	size_t canLengthIndex;
	size_t variantIndex;
	size_t varLengthIndex;
	int initialCanLength;
	int initialVarLength;
	const char *initialCanonical;
	const char *initialVariant;
	char *table;
};


#define MEMUNDEFINED 0
#define MEMFALSE 1
#define MEMTRUE 2


//For memoization, use the lookup table convention that 0 == not yet computed, 1 == flase, 2 == true.
//This way, any non-zero result can be right shifted to get the appropriate value.
char memoizedStrictBinaryVariantAddition(const char *canonical, const int canLength, const char *variant, const int varLength, const additionData *data)
{

	char *temp = data->table + (data->canonicalIndex*	(canonical - data->initialCanonical)
							+ data->canLengthIndex*	(canLength-1)
							+ data->variantIndex*	(variant - data->initialVariant)
							+						(varLength-1));
	if (*temp)
	{
		return (*temp >> 1);//This shift is equivilent to (*temp - 1) in the range [1 to 2]
	}

	if (varLength > canLength)
	{
		if (canonical[0] != variant[0] || canonical[canLength - 1] != variant[varLength - 1])
		{
			*temp = MEMFALSE;
			return false;
		}

		int innerLength = varLength / 2;

		if (varLength % 2 == 0
			&& compareArrays(variant, variant + innerLength, innerLength)
			&& memoizedStrictBinaryVariantAddition(canonical, canLength, variant, innerLength, data)) /*Half point Sub-strings exist and match*/
		{
			//Recursivly perform variant addition on one of the substrings
			*temp = MEMTRUE;
			return true;
		}
		else if (canLength > 1)/*Half-point recursion was not possible or didn't return a positive result*/
		{
			//If possible (i.e. canLength > 1), perform split, running through variant splits from ([0,1] [1,varLength]) to ([0,varLength-1] [varLength-1,varLength]), and -for each variant split- run through all possible canonical splits and recurse through
			for (int varSplit = 1; varSplit < varLength; ++varSplit)
			{
				//int upperSplit = canLength < varSplit + 1 ? canLength : varSplit + 1;
				int upperSplit = min(canLength, varSplit + 1);
				//int current = canLength - 1 < varLength - varSplit ? canLength - 1: varLength - varSplit;
				int current = min(canLength - 1, varLength - varSplit);
				for (int canSplit = canLength - current; canSplit < upperSplit; ++canSplit)
				{
					if (canSplit <= canLength/2)
					{
						if (memoizedStrictBinaryVariantAddition(canonical, canSplit, variant, varSplit, data) && memoizedStrictBinaryVariantAddition(canonical + canSplit, canLength - canSplit, variant + varSplit, varLength - varSplit, data))
						{
							*temp = MEMTRUE;
							return 1;
						}
					}
					else
					{
						if (memoizedStrictBinaryVariantAddition(canonical + canSplit, canLength - canSplit, variant + varSplit, varLength - varSplit, data) && memoizedStrictBinaryVariantAddition(canonical, canSplit, variant, varSplit, data))
						{
							*temp = MEMTRUE;
							return 1;
						}
					}
				}
			}
		}

		*temp = MEMFALSE;
		return false;
	}
	else
	{
		char retVal = (char)equalString(canonical, canLength, variant, varLength);
		*temp = retVal + 1;
		return retVal;
	}
}

bool memoizedStrictBinaryVariantAdditionInit(const char *canonical, const int canLength, const char *variant, const int varLength, char *error)
{
	if (canLength > 0 && varLength > 0)//Only perform the algorithm if there would be enough space to memoize, otherwise return true only if canLength and varLength are both 0
	{
		if (canonical[0] != variant[0] || canonical[canLength - 1] != variant[varLength - 1])
			return false;

		//Perform preliminary search
		int canIndex = 0;
		int varIndex = 0;

		while (varIndex < varLength - 1 && canIndex < canLength - 1)
		{

			if (canonical[canIndex] == variant[varIndex] && canonical[canIndex + 1] == variant[varIndex + 1])
			{
				canIndex++;
			}

			++varIndex;
		}

		if (canIndex < canLength - 1)
			return false;

		//size_t tableSizeTemp = canLength*canLength*varLength*varLength;

		additionData data;

		data.initialCanonical = canonical;
		data.initialCanLength = canLength;
		data.initialVariant = variant;
		data.initialVarLength = varLength;
		data.tableSize = canLength*canLength*varLength*varLength;;
		data.canonicalIndex = canLength*varLength*varLength;
		data.canLengthIndex = varLength*varLength;
		data.variantIndex = varLength;
		data.varLengthIndex = 1;
		data.table = (char *)calloc(data.tableSize, sizeof(char));//Calloc A

		bool returnVal = (bool)memoizedStrictBinaryVariantAddition(canonical, canLength, variant, varLength, &data);

		free(data.table);//Free A

		return returnVal;
	}
	else
	{
		return canLength == varLength;
	}
}

#define ALPHALENGTH 256

struct canonicalLink
{
	canonicalLink *next;
	int place;
};

bool scanningStrictBinaryVariantAddition(const char *canonical, const int canLength, const char *variant, const int varLength, char *error)
{

	if (canLength > 0 && varLength > 0)//Only perform the algorithm if there would be enough space, otherwise return true only if canLength and varLength are both 0
	{
		if (canonical[0] != variant[0] || canonical[canLength - 1] != variant[varLength - 1])
			return false;

		//Perform preliminary search
		int canIndex = 0;
		int varIndex = 0;

		while (varIndex < varLength - 1 && canIndex < canLength - 1)
		{

			if (canonical[canIndex] == variant[varIndex] && canonical[canIndex + 1] == variant[varIndex + 1])
			{
				canIndex++;
			}

			++varIndex;
		}

		if (canIndex < canLength - 1)
			return false;



		int collectionTable[ALPHALENGTH];
		int maxOccurance = 0;
		int totalLinks = 0;
		memset(collectionTable, 0, sizeof(collectionTable));

		//Creats an array counting the occurances of all the possible characters while tracking the max value
		for (int ii = 0; ii < canLength; ++ii)
		{
			collectionTable[canonical[ii]] += 1;
		}

		for (int jj = 0; jj < ALPHALENGTH; ++jj)
		{
			if (collectionTable[jj] > maxOccurance)
				maxOccurance = collectionTable[jj];

			if (collectionTable[jj] > 1)
				totalLinks += (collectionTable[jj] - 1);
		}

		//Generate preliminary canon table
		int *prelimTable = (int*)calloc(canLength, sizeof(int));//ALLOCATE A
		//memset(prelimTableMemory, 0, ALPHALENGTH*canLength*sizeof(canonicalLink*));
		//void *allocationPoint = prelimTableMemory;

		int precursorTable[ALPHALENGTH];
		memset(precursorTable, 0, sizeof(precursorTable));

		for (int kk = 0; kk < canLength; ++kk)
		{
			if (precursorTable[canonical[kk]])
			{
				//Use the existing entry in the precursor table as the current preliminary backreference, then replace it
				prelimTable[kk] = precursorTable[canonical[kk]];
			}
			else
			{
				//Put the index in the precursor table
				precursorTable[canonical[kk]] = kk;
			}
		}


		
		free(prelimTable); //FREE A
	}
	else
	{
		return canLength == varLength;
	}
}

bool strMSBVariantAddition(const char *canonical, const char *variant, char *error)
{
	return memoizedStrictBinaryVariantAdditionInit(canonical, strlen(canonical), variant, strlen(variant), error);
}



bool printTruth(bool val)
{
	printf(val ? "true\n" : "false\n");
	return val;
}

bool printFalse(bool val)
{
	printf(val ? "false\n" : "true\n");
	return val;
}

void printAll(const char *canonical, const char *variant, char *error)
{
	//printf("Lax -\n");
	//printTruth(strVariantAddition(canonical, variant, error));
	printf("Memoized Strict Binary -\n");
	printTruth(strMSBVariantAddition(canonical, variant, error));
	printf("Strict Binary -\n");
	printTruth(strSBVariantAddition(canonical, variant));
}

int main(int argc, char *argv[])
{
	char err = 0;
	printAll("", "", &err);
	printAll("n", "nnnnnnnnnnnn", &err);
	printAll("nnn", "nnnnnnnnnnn", &err);
	printAll("abca", "abca", &err);
	printAll("abca", "abcbcabcbca", &err);
	printAll("abca", "abcbcaabcbca", &err);
	printAll("abcd", "abcdabcd", &err);
	printAll("abcd", "abcdababcd", &err);
	printAll("abcd", "abcdccd", &err);
	printAll("abca", "abcbcacbcbca", &err);

	printAll("j", "", &err);
	printAll("", "j", &err);
	printAll("nnnn", "nnn", &err);
	printAll("abca", "acba", &err);
	printAll("abcd", "abcdabcda", &err);

	printAll("adfhghsdfhghsdfhsdfas", "adfffhjksdfsdfdfsdffhjksdfsdfsdfksdfsdfsdfhf", &err);

	printf("\nPlease enter a canonical sequence:\n");

	const int LineSize = 256;

	char canonLine[LineSize];
	char varyLine[LineSize];

	if (fgets(canonLine, sizeof(canonLine), stdin))
	{
		printf("Please enter a variant sequence:\n");
		if (fgets(varyLine, sizeof(varyLine), stdin))
		{
			printTruth(strMSBVariantAddition(canonLine, varyLine, &err));
			printTruth(strSBVariantAddition(canonLine, varyLine));
			printTruth(strVariantAddition(canonLine, varyLine, &err));
		}
		else
		{
			err |= BAD_INPUT;
		}
	}
	else
	{
		err |= BAD_INPUT;
	}

	if (0 != err)
		printf("Error %i occured!", err);

	getchar();

	return err;
}
