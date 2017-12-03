#include<stdio.h>
#include<stdlib.h>

//type substitution
typedef int BOOL;

//Errors
const char OOM = 1;//Out Of Memory
const char BAD_INPUT = 2;

//Bools
const char true = 1;
const char false = 0;

BOOL compareArrays(const char *first, const char *second, int length)
{
	for (int ii = 0; ii < length; ++ii)
	{
		if (first[ii] != second[ii])
			return false;
	}
	
	return true;
}

BOOL equalString(const char *canonical, int canLength, const char *variant, int varLength)
{
	if (canLength != varLength)
		return false;
	else
		return compareArrays(canonical, variant, canLength);
}

BOOL stringDiff(const char *canonical, int canLength, const char *variant, int varLength)
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


BOOL variantAdditionResInner(const char *canonical, int canLength, char *variant, int varLength)
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


BOOL variantAdditionInner(const char *canonical, int canLength, char *variant, int varLength)
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
				if (compareArrays(variant + index, variant + subIndex, innerLength))
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
				if (compareArrays(variant + reverseIndex, variant + reverseSubIndex, innerLength))
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

BOOL variantAddition(const char *canonical, int canLength, const char *variant, int varLength, char *error)
{
	if (canLength > varLength)
	{
		return false; //Prevents varlLength from not being greater than canLength, causing a call to malloc that would be equal or less than 0
	}
	else
	{
		//Allocate (n^2 + n)/2 - (m^2 + m)/2 bytes, where n == varLength and m == (canLength - 1), which should be the maximum needed size given varLength decreases by at least 1 each recursion
		const int minorLength = canLength - 1;
		char *newBuffer = malloc((varLength*varLength + varLength - minorLength*minorLength - minorLength) * sizeof(char) / 2);

		if (newBuffer)
		{
			for (int ii = 0; ii < varLength; ++ii)
			{
				newBuffer[ii] = variant[ii];
			}

			int val = variantAdditionInner(canonical, canLength, newBuffer, varLength);

			free(newBuffer);

			return val;
		}
		else
		{
			*error |= OOM;
			return false;
		}
	}
}

BOOL strVariantAddition(const char *canonical, const char *variant, char *error)
{
	return variantAddition(canonical, strlen(canonical), variant, strlen(variant), error);
}


//Variant expects a buffer containing varLength^2 bytes of memory to be passed to it
BOOL strictBinaryVariantAddition(const char *canonical, int canLength, const char *variant, int varLength)
{
	if (varLength > canLength)
	{		
		int innerLength = varLength/2;
		
		if (varLength % 2 == 0
			&& compareArrays(variant, variant+innerLength, innerLength)
			&& strictBinaryVariantAddition(canonical, canLength, variant, innerLength)) /*Half point Sub-strings exist and match*/
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
				for (int canSplit = 1; canSplit < canLength; ++canSplit)
				{
					if (strictBinaryVariantAddition(canonical, canSplit, variant, varSplit) && strictBinaryVariantAddition(canonical+canSplit, canLength-canSplit, variant+varSplit, varLength-varSplit))
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

BOOL strSBVariantAddition(const char *canonical, const char *variant)
{
	return strictBinaryVariantAddition(canonical, strlen(canonical)-1, variant, strlen(variant)-1);
}


BOOL printTruth(BOOL val)
{
	printf(val ? "true\n" : "false\n");
	return val;
}

BOOL printFalse(BOOL val)
{
	printf(val ? "false\n" : "true\n");
	return val;
}

#define LINESIZE 256

int main(int argc, char *argv[])
{
	char err = 0;
	printTruth(strVariantAddition("", "", &err));
	printTruth(strVariantAddition("n", "nnnnnnnnnnnn", &err));
	printTruth(strVariantAddition("nnn", "nnnnnnnnnnn", &err));
	printTruth(strVariantAddition("abca", "abca", &err));
	printTruth(strVariantAddition("abca", "abcbcabcbca", &err));
	printTruth(strVariantAddition("abca", "abcbcaabcbca", &err));
	printTruth(strVariantAddition("abcd", "abcdabcd", &err));
	printTruth(strVariantAddition("abcd", "abcdababcd", &err));
	printTruth(strVariantAddition("abcd", "abcdccd", &err));
	printTruth(strVariantAddition("abca", "abcbcacbcbca", &err));

	printFalse(strVariantAddition("j", "", &err));
	printFalse(strVariantAddition("", "j", &err));
	printFalse(strVariantAddition("nnnn", "nnn", &err));
	printFalse(strVariantAddition("abca", "acba", &err));
	printFalse(strVariantAddition("abcd", "abcdabcda", &err));

	printf("\nPlease enter a canonical sequence:\n");

	char canonLine[LINESIZE];
	char varyLine[LINESIZE];

	if (fgets(canonLine, sizeof(canonLine), stdin))
	{
		printf("Please enter a variant sequence:\n");
		if (fgets(varyLine, sizeof(varyLine), stdin))
		{
			printf("Lax Variant Addition:\n");
			printTruth(strVariantAddition(canonLine, varyLine, &err));
			printf("Strict Variant Addition:\n");
			printTruth(strSBVariantAddition(canonLine, varyLine));
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
