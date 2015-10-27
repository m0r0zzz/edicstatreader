#include "Main.hpp"

using namespace std;

uint32_t globend, operend;



void showGlob(const stat_glob_rec rec, ostream* out){
	/*sys_uart_write('{');
	sys_uart_print_dword(rec.signature);
	sys_uart_write(','); sys_uart_write(' ');
	sys_uart_print_word(rec.status);
	sys_uart_write(','); sys_uart_write(' ');
	sys_uart_print_dword(rec.ctrl);
	sys_uart_write(','); sys_uart_write(' ');
	sys_uart_printdec(rec.timer);
	sys_uart_write('}');*/

	*out << "\tTime: " << std::dec << (float)rec.timer/32768.0 << "sec.\n";
	*out << "\tSTATUS: " << std::hex << rec.status << "\n";
	*out << "\tCTRL: " << std::hex << rec.ctrl << "\n";
	*out << "\tErrorCode: " << std::dec << rec.errc << "\n";
	*out << "\tReturnAddress: " << std::hex << rec.link << "\n";
	*out << "\tStackTop: " << std::hex << rec.stack << "\n";
	*out << "\tErrData1: " << std::hex << rec.data1 << "\n";
	*out << "\tErrData2: " << std::hex << rec.data2 << "\n";
	*out << "\n";
}

void showBad(ostream* out){
    *out << "Bad Record\n\n";
}

void showOper(const stat_oper_rec rec, ostream* out){
	/*sys_uart_write('{');
	sys_uart_print_dword(rec.signature);
	sys_uart_write(','); sys_uart_write(' ');
	sys_uart_printdec(rec.timer);
	sys_uart_write('}');*/
	*out << "\tTime: " << std::dec << (float)rec.timer/32768.0 << "sec.\n";
	*out << "\tSavedFrames: " << std::dec << rec.frm_saved << "\n";
	*out << "\tCapturedFrames: " << std::dec << rec.frm_captured << "\n";
	*out << "\tFlashUsed: " << std::dec << rec.nandaddr << '(' << std::hex << rec.nandaddr << ") pages\n";
    *out << "\n";
}

template<class T>
T ptr_major_vote(const T *ptrs){
	uint8_t cnt[PTRNUM] = { 0 };
	uint8_t maximum = 0;
	T ret = 0;
	for(uint8_t i = 0; i < PTRNUM; i++) for(uint8_t j = i; j < PTRNUM; j++){
			if(ptrs[i] == ptrs[j]) cnt[i]++, cnt[j]++;
	}
	for(uint8_t i = 0; i < PTRNUM; i++) if(cnt[i] > maximum) maximum = cnt[i], ret = ptrs[i];
	if(maximum == 1) return 0;
	return ret;
}

void loadPtrs(uint8_t* buffer){
    stat_header hdr;
    memcpy(&hdr,buffer + HDR_ADDR,RECSIZE);
    if(hdr.signature != HDR_SIGN) throw runtime_error("Bad header signature");
    globend = ptr_major_vote(hdr.globend);
    operend = ptr_major_vote(hdr.operend);

    if(globend == 0 || operend == 0) throw runtime_error("Corrupted stat pointers");

    cout << "Soft-requested reboots: " << std::dec << (unsigned int)hdr.rebootcnt << "\n";

    uint32_t nandaddr = ptr_major_vote(hdr.nandaddr);
    if(nandaddr != 0) cout << "Last written NAND macropage: " << std::hex << nandaddr << "\n\n";
    else cout << "\n";

}

void traverseGlob(uint8_t* buffer, ostream* out){
	stat_glob_rec rec;
	uint16_t adr = 0;
	for(uint16_t addr = GLOB_ADDR; addr < GLOB_ADDR+GLOB_IMM_NUM*RECSIZE; addr+=RECSIZE){
//		spimem_read(addr  , RECSIZE, &rec);
        memcpy(&rec,buffer + addr,RECSIZE);
		if(rec.signature != GLOB_SIGN){
            showBad(out);
            continue;
		}
		showGlob(rec, out);
	}

//	sys_uart_write('|');

	adr = globend+RECSIZE;
	if(adr > GLOB_ADDR + GLOB_IMM_NUM*RECSIZE){
        do{
            if(adr >= GLOB_END_ADDR) adr = GLOB_ADDR + GLOB_IMM_NUM*RECSIZE;
            //spimem_read(adr, RECSIZE, &rec);
            memcpy(&rec,buffer + adr ,RECSIZE);
            adr += RECSIZE;
            if(rec.signature != GLOB_SIGN){
                showBad(out);
                continue;
            }
            showGlob(rec, out);
        } while(adr != globend+RECSIZE);
	}

//	sys_uart_write(' ');
}

void traverseOper(uint8_t* buffer, ostream* out){
	stat_oper_rec rec;
	uint16_t adr = 0;

//	sys_uart_write('O');

	for(uint16_t addr = OPER_ADDR; addr < OPER_ADDR+OPER_IMM_NUM*RECSIZE; addr+=RECSIZE){
		//spimem_read(addr, RECSIZE, &rec);
		memcpy(&rec,buffer+ addr,RECSIZE);
		if(rec.signature != OPER_SIGN){
            showBad(out);
            continue;
		}
		showOper(rec, out);
	}

//	sys_uart_write('|');

	adr = operend + RECSIZE;
	if(adr > OPER_ADDR + OPER_IMM_NUM*RECSIZE){
        do{
            if(adr >= OPER_END_ADDR) adr = OPER_ADDR + OPER_IMM_NUM*RECSIZE;
            //spimem_read(adr, RECSIZE, &rec);
            memcpy(&rec,buffer + adr,RECSIZE);
            adr+=RECSIZE;
            if(rec.signature != OPER_SIGN){
                showBad(out);
                continue;
            }
            showOper(rec, out);
        } while(adr != operend+RECSIZE);
	}

//	sys_uart_write(' ');
}


void printHelp(char* arg0){
    cout << "Log/Statistics binary-to-readable conversion utility.\nUsage:\n\t" << arg0 << " <infile> [outfile]\n";
    cout << "If outfile isn't supplied, stdout is used" << endl;
}

int main(int argc, char** argv){
    if(argc != 2 && argc != 3){
        printHelp(argv[0]);
        return 0;
    }

    ifstream in(argv[1], std::ios::binary);
    if(!in.is_open()){
        cout << "Can't open infile" << endl;
        return -1;
    }
    ostream* out;
    if(argc == 3){
        ofstream* myout = new ofstream(argv[2]);
        if(!myout->is_open()){
            cout << "Can't open outfile" << endl;
            delete myout;
            return -2;
        }
        out = myout;
    }
    else out = &cout;

    uint8_t buffer[16384] = { 0 };

    in.read((char *)buffer, 16384);
    in.close();

    loadPtrs(buffer);

    *out << "Global stats:\n";
    traverseGlob(buffer, out);
    *out << "\nOperative stats:\n";
    traverseOper(buffer, out);

    *out << "END." << endl;

    if(argc == 3) delete out;

    return 0;
}
