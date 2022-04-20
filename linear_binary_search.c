#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#define MAX_YEAR_DURATION	10	// 기간
#define LINEAR_SEARCH 0
#define BINARY_SEARCH 1

// 구조체 선언
typedef struct {
	char	name[20];		// 이름
	char	sex;			// 성별 M or F
	int		freq[MAX_YEAR_DURATION]; // 연도별 빈도
} tName;

typedef struct {
	int		len;		// 배열에 저장된 이름의 수
	int		capacity;	// 배열의 용량 (배열에 저장 가능한 이름의 수)
	tName	*data;		// 이름 배열의 포인터
} tNames;

////////////////////////////////////////////////////////////////////////////////
// 함수 원형 선언(declaration)

// 연도별 입력 파일을 읽어 이름 정보(이름, 성별, 빈도)를 이름 구조체에 저장
// 이미 구조체에 존재하는(저장된) 이름은 해당 연도의 빈도만 저장
// 새로 등장한 이름은 구조체에 추가
// 주의사항: 동일 이름이 남/여 각각 사용될 수 있으므로, 이름과 성별을 구별해야 함
// names->capacity는 2배씩 증가
// 선형탐색(linear search) 버전
void load_names_lsearch( FILE *fp, int year_index, tNames *names);

// 이진탐색(binary search) 버전
void load_names_bsearch( FILE *fp, int year_index, tNames *names);

// 구조체 배열을 화면에 출력
void print_names( tNames *names, int num_year);

// qsort를 위한 비교 함수
// 정렬 기준 : 이름(1순위), 성별(2순위)
int compare( const void *n1, const void *n2);

////////////////////////////////////////////////////////////////////////////////
// 함수 정의 (definition)

// 이름 구조체를 초기화
// len를 0으로, capacity를 1로 초기화
// return : 구조체 포인터
tNames *create_names(void)
{
	tNames *pnames = (tNames *)malloc( sizeof(tNames));
	
	pnames->len = 0;
	pnames->capacity = 1;
	pnames->data = (tName *)malloc(pnames->capacity * sizeof(tName));

	return pnames;
}

// 이름 구조체에 할당된 메모리를 해제
void destroy_names(tNames *pnames)
{
	free(pnames->data);
	pnames->len = 0;
	pnames->capacity = 0;

	free(pnames);
}

////////////////////////////////////////////////////////////////////////////////
int main(int argc, char **argv)
{
	tNames *names;
	int mode;  // search mode
	
	FILE *fp;
	int num_year = 0;
	
	if (argc <= 2)
	{
		fprintf( stderr, "Usage: %s mode FILE...\n\n", argv[0]);
		fprintf( stderr, "mode\n\t-l\n\t\twith linear search\n\t-b\n\t\twith binary search\n");
		return 1;
	}
	
	if (strcmp( argv[1], "-l") == 0) mode = LINEAR_SEARCH;
	else if (strcmp( argv[1], "-b") == 0) mode = BINARY_SEARCH;
	else {
		fprintf( stderr, "unknown mode : %s\n", argv[1]);
		return 1;
	}
	
	// 이름 구조체 초기화
	names = create_names();

	// 첫 연도 알아내기 "yob2009.txt" -> 2009
	int start_year = atoi( &argv[2][strlen(argv[2])-8]);
	
	for (int i = 2; i < argc; i++)
	{
		num_year++;
		fp = fopen( argv[i], "r");
		if( !fp) {
			fprintf( stderr, "cannot open file : %s\n", argv[i]);
			return 1;
		}

		int year = atoi( &argv[i][strlen(argv[i])-8]); // ex) "yob2009.txt" -> 2009
		
		fprintf( stderr, "Processing [%s]..\n", argv[i]);
		
		if (mode == LINEAR_SEARCH)
		{
			// 연도별 입력 파일(이름 정보)을 구조체에 저장
			// 선형탐색 모드
			load_names_lsearch( fp, year-start_year, names);
		
		}
		else if (mode == BINARY_SEARCH)
		{
            // 이진탐색 모드
			load_names_bsearch( fp, year-start_year, names);
            
            // 정렬 (이름순 (이름이 같은 경우 성별순))
            qsort( names->data, names->len, sizeof(tName), compare);
		}
		fclose( fp);

	}
	
	// 정렬 (이름순 (이름이 같은 경우 성별순))
	qsort( names->data, names->len, sizeof(tName), compare);
    
	// 이름 구조체를 화면에 출력
	print_names( names, num_year);

	// 이름 구조체 해제
	destroy_names( names);
	
	return 0;
}


/////////////////////////////////////////////////////////////////////////

// 선형탐색(linear search) 버전
void load_names_lsearch( FILE *fp, int year_index, tNames *names)
{
    
    char temp[30];
    int i;
    
    while(fgets(temp, sizeof(temp), fp) != NULL)
    {
        /* temp에 문자열 한 줄 저장했음. (while문 내에서) */
        
        char tempName[20];
        char tempSex;
        
        /* temp의 이름,성별,빈도 에서 tempName에 이름 따로 저장, tempSex에 성별 따로 저장 */
        sscanf(temp, "%[^',']", tempName);
        sscanf(temp, "%*[^','],%[^',']", &tempSex);
        
        
        /*
         if문으로 이미 구조체에 있는 이름, 성별 파악
         temp에 있는 문자열에서 이름, 성별 부분만 떼내서 if문으로 이미 구조체에 있는지 파악하고 비교
         tempName이 파일에서 읽은 이름 주소, (names->data).name이 함수에서 주어진거에서 읽은 이름 주소(구조체에 저장된 모든 이름)
         
         for loop 안에서 이미 동일한 이름&성별 pair가 있는 지 확인하고 if문에서 있는 걸로 잡히면 index를 따로 저장해서
         for 문 밖에서 빈도를 올리고
         for loop에서 없는 것으로 나오면
         새로 구조체에 이름, 성별, 빈도 저장
         
         for문에서 이름의 [i]번째를 추출
         */
        
        int idx=-1;
        
        for(i=0; i<names->len; i++)
        {
            if((strcmp(tempName, names->data[i].name) == 0) && (tempSex == names->data[i].sex))
            {
                idx = i;
                break;
            }
        }
        
        if(idx == -1)
        {
            // memset name->len 번째 주솟값.

            memset(&(names->data[names->len]), 0, sizeof(tName));
            sscanf(temp, "%[^','],%[^','],%d", names->data[names->len].name, &(names->data[names->len]).sex, &(names->data[names->len].freq[year_index]));
            (names->len)++;
            
        }
        else
        {
            sscanf(temp, "%*[^','],%*[^','],%d", &((names->data)[idx].freq[year_index]));
            continue;
        }
        
        
        /*
         사이클 한번 순환할 때마다 동적 메모리 용량 체크하고(if문) 꽉 찼으면 2배로 증가
         한 줄 저장한 후에 용량 체크하고 2배로 capacity 늘려주기 필요 - realloc
         */
        //만약 capacity가 names.data 크기보다 작으면
        // 저장된 용량이랑 저장가능 용량이 같아지면 추가하면 된다.
        
        if( (names->len) == (names->capacity) )
        {
            (names->capacity)*=2;
            names->data = realloc(names->data, sizeof(tName)*(names->capacity)); // (tName *)
        }
        
    }
    
}



// 이진탐색(binary search) 버전
void load_names_bsearch( FILE *fp, int year_index, tNames *names){
    
    char temp[30];
    
    int mid = (names->len)/2;
    
    while(fgets(temp, sizeof(temp), fp) != NULL) // 파일을 한줄씩 읽어서 한 줄씩 temp에 저장. 마지막줄까지 읽는다.
    {
        
        /*
         bsearch 함수로 구조체에 해당 이름, 성별이 있는지 비교하고
         없으면 구조체 맨 뒤에 이름, 성별, 빈도수를 추가해주고
         있으면 해당 연도의 빈도수만 갱신해준다.
         */
        
        tName* temp_tName = (tName*)malloc(sizeof(tName));
        sscanf(temp, "%[^','],%[^',']", temp_tName->name, &(temp_tName->sex));
        
        tName* ptr = (tName*)bsearch(temp_tName, names->data, names->len, sizeof(names->data[0]), compare);
        
        free(temp_tName);
        
        // ptr에는 검색된 값에 대한 주소 값 반환. 검색되지 않았다면 NULL값 반환.
        //ptr = bsearch( temp, names->data, sizeof(names->len), sizeof(names->data), compare);
        
        
        if(ptr == NULL)  // ptr이 NULL이라면, 이름이 없으니 이름, 성별, 빈도수 저장.
        {
            memset(&(names->data[names->len]), 0, sizeof(tName));
            sscanf(temp, "%[^','],%[^','],%d", names->data[names->len].name, &(names->data[names->len]).sex, &(names->data[names->len].freq[year_index]));
            (names->len)++;
        }
        else if(ptr != NULL) // ptr이 NULL이 아니라면, 이미 이름이 있으니 빈도수 저장.
        {
            sscanf(temp, "%*[^','],%*[^','],%d", &ptr->freq[year_index]);  // ? 몇번째에 넣는걸까? ptr 주소 값만 가지고 몇번째에 빈도 값을 갱신하는지 알 수 있나? idx값?
            continue;
        }
        
        if( (names->len) == (names->capacity) )
        {
            (names->capacity)*=2;
            names->data = realloc(names->data, sizeof(tName)*(names->capacity)); // (tName *)
        }
        
    }
    
}






// 구조체 배열을 화면에 출력
void print_names( tNames *names, int num_year)
{
    
    int i, j;
    
    for(i=0; i <= names->len; i++)
    {
        printf("%s\t%c\t", names->data[i].name, names->data[i].sex);
        
        for(j=0; j<num_year; j++)
        {
            printf("%d\t", names->data[i].freq[j]);
        }
        printf(" \n");
    }
    
}



// qsort를 위한 비교 함수
// 정렬 기준 : 이름(1순위), 성별(2순위)
/*
 1순위와 2순위를 어떻게 구별해서 정렬할까?
 이름은 정렬하는데 2순위 성별은 어떻게 정렬?
 아래대로 비교하면 자연스럽게 정렬 되는건지?
 */

int compare( const void *n1, const void *n2)
{
    tName* a = (tName*)n1;
    tName* b = (tName*)n2;
    
    if (!strcmp(a->name, b->name)) { // µŒ ¿Œ¿⁄¿« ¿Ã∏ß¿Ã ∞∞¿∫ ∞ÊøÏ
        if (a->sex > b->sex) return 1;
        else if (a->sex == b->sex) return 0;
        else return -1;
    }
    else return(strcmp(a->name, b->name));
}



