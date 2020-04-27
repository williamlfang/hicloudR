/*
* @Author: “williamlfang”
* @Date:   2019-08-26 18:48:08
* @Last Modified 2019-09-09
* @Last Modified time: 2020-04-27 17:30:35
*/

/*
 * 设置编译环境
 * touch ~/.R/Makevars
 * -------------------
 *
   CC=/opt/local/bin/gcc-mp-4.7
   CXX=/opt/local/bin/g++-mp-4.7
   CPLUS_INCLUDE_PATH=/opt/local/include:$CPLUS_INCLUDE_PATH
   LD_LIBRARY_PATH=/opt/local/lib:$LD_LIBRARY_PATH
   CXXFLAGS= -g0 -O2 -Wall
   MAKE=make -j4

## for C code
CFLAGS=               -O3 -g0 -Wall -pipe -pedantic -std=gnu99

## for C++ code
#CXXFLAGS=             -g -O3 -Wall -pipe -Wno-unused -pedantic -std=c++11
CXXFLAGS=             -g -O3 -Wall -pipe -Wno-unused -pedantic

## for Fortran code
#FFLAGS=-g -O3 -Wall -pipe
FFLAGS=-O3 -g0 -Wall -pipe
## for Fortran 95 code
#FCFLAGS=-g -O3 -Wall -pipe
FCFLAGS=-O3 -g0 -Wall -pipe

# VER=-4.8
# CC=ccache gcc$(VER)
# CXX=ccache g++$(VER)
# SHLIB_CXXLD=g++$(VER)
FC=ccache gfortran
F77=ccache gfortran
MAKE=make -j8

 *
 * -------------------
 */

// =============================================================================
// -------------------------
#include <fstream>
#include <sstream>
#include <string>
#include <Rcpp.h>

using namespace std;
using namespace Rcpp;
// Enable C++11 via this plugin (Rcpp 0.10.3 or later)
// [[Rcpp::plugins(cpp11)]]
// -------------------------

/* -------------------------
 * 通用的格式如下

// [[Rcpp::export]]
RETURN_TYPE FUNCTION_NAME(ARGUMENT_TYPE ARGUMENT){

    //do something

    return RETURN_VALUE;
}

*/
// =============================================================================

// -----------------------------------------------------------------------------
// [[Rcpp::export]]
int rcpp_test() {
    return 1;
}
// -----------------------------------------------------------------------------




// -----------------------------------------------------------------------------
/*
- rcpp_readFile(datafile):

fast reading datafile
Ref: https://gist.github.com/hadley/6353939
*/
// [[Rcpp::export]]
CharacterVector rcpp_readFile(std::string path) {
    std::ifstream in(path.c_str());
    std::string contents;
    in.seekg(0, std::ios::end);
    contents.resize(in.tellg());
    in.seekg(0, std::ios::beg);
    in.read(&contents[0], contents.size());
    in.close();
    return(contents);
}
// -----------------------------------------------------------------------------


// -----------------------------------------------------------------------------
/*
- rcpp_updateCalendar:
更新 calendar
*/
// [[Rcpp::export]]
DataFrame rcpp_updateCalendar(DataFrame calendar) {
    int n = calendar.nrow();
    StringVector SourceID = calendar["SourceID"];
    StringVector DataPath = calendar["DataPath"];

    for ( int i = 1; i < n; ++i ) {
        if ( StringVector::is_na( SourceID[i] ) ) {
            SourceID[i] = SourceID[i-1];
            DataPath[i] = DataPath[i-1];
        }
    }

    StringVector days = calendar["days"];
    StringVector years( n );
    for ( int i = 0; i < n; ++i ) {
        years[i] = string( days[i] ).substr( 0,4 );
    }

    DataFrame df = DataFrame::create(
        Named( "years" ) = years,
        Named( "days" ) = calendar["days"],
        Named( "nights" ) = calendar["nights"],
        Named( "SourceID" ) = SourceID,
        Named( "DataPath" ) = DataPath,
        _["stringsAsFactors"] = false
    );

    return df;
}
// -----------------------------------------------------------------------------


// -----------------------------------------------------------------------------
/*
- rcpp_updateNext:
更新 下一个 NA 的数据
*/
// [[Rcpp::export]]
StringVector rcpp_updateNext(StringVector vecOld) {
    int n = vecOld.size();
    StringVector vecNew = clone( vecOld );

    for ( int i=1; i<n; ++i ) {
        if ( StringVector::is_na( vecNew[i] ) ) {
            vecNew[i] = vecNew[i-1];
        }
    }

    return vecNew;
}
// -----------------------------------------------------------------------------


// -----------------------------------------------------------------------------
/*
- rcpp_tick_data_del_na:
清洗 NA 数据
*/
// [[Rcpp::export]]
void rcpp_tick_data_del_na(DataFrame dt) {
    const int NROW = dt.nrow();

    NumericVector
        open = dt["OpenPrice"],
        high = dt["HighestPrice"],
        low = dt["LowestPrice"],
        close = dt["ClosePrice"],
        last = dt["LastPrice"],

        askprice1 = dt["AskPrice1"],
        askvolume1 = dt["AskVolume1"],
        bidprice1 = dt["BidPrice1"],
        bidvolume1 = dt["BidVolume1"],

        upperlimit = dt["UpperLimitPrice"],
        lowerlimit = dt["LowerLimitPrice"],

        volume = dt["Volume"],
        turnover = dt["Turnover"],
        openinterest = dt["OpenInterest"],
        avg = dt["AveragePrice"];

    const long double MAX = 1e300;

    for ( int i=0; i<NROW; ++i ) {
        if ( open[i] > MAX ) open[i] = NA_REAL;
        if ( high[i] > MAX ) high[i] = NA_REAL;
        if ( low[i] > MAX ) low[i] = NA_REAL;
        if ( close[i] > MAX ) close[i] = NA_REAL;
        if ( last[i] > MAX ) last[i] = NA_REAL;

        if ( askprice1[i] > MAX ) askprice1[i] = NA_REAL;
        if ( askvolume1[i] > MAX ) askvolume1[i] = NA_INTEGER;
        if ( bidprice1[i] > MAX ) bidprice1[i] = NA_REAL;
        if ( bidvolume1[i] > MAX ) bidvolume1[i] = NA_INTEGER;

        if ( upperlimit[i] > MAX ) upperlimit[i] = NA_REAL;
        if ( lowerlimit[i] > MAX ) lowerlimit[i] = NA_REAL;

        if ( volume[i] > MAX ) volume[i] = NA_INTEGER;
        if ( turnover[i] > MAX ) turnover[i] = NA_REAL;
        if ( openinterest[i] > MAX ) openinterest[i] = NA_INTEGER;
        if ( avg[i] > MAX ) avg[i] = NA_REAL;

        if ( ( turnover[i] == 0 || NumericVector::is_na( turnover[i] ) ) &&
             ( avg[i] != 0 && !NumericVector::is_na( avg[i]) ) )
            turnover[i] = volume[i] * avg[i];
    }
}
// -----------------------------------------------------------------------------


// -----------------------------------------------------------------------------
/*
- rcpp_tick_data_add_time:
增加 时间信息
*/
// [[Rcpp::export]]
void rcpp_tick_data_add_time_old_version(DataFrame dt) {
    const int NROW = dt.nrow();

    StringVector
        TimeStamp = dt["TimeStamp"],
        UpdateTime = dt["UpdateTime"];

    NumericVector
        hh = dt["UpdateHour"],
        mm = dt["UpdateMinute"],
        ss = dt["UpdateSecond"],
        ms = dt["UpdateMilliSec"],
        ts = dt["NumeriTimeStamp"],
        ex = dt["NumericExchTime"];

    // 转化时间
    string s;
    int k;
    // UpdateTime = 18:35:39.500
    //              0: 3  6  9
    // TimeStamp = 20180531:20:58:04:852904
    //                      9  12 15 18           
    for ( int i=0; i<NROW; ++i ) {
        s = string( UpdateTime[i] );
        hh[i] = stoi( s.substr( 0,2 ) );
        mm[i] = stoi( s.substr( 3,2 ) );
        ss[i] = stoi( s.substr( 6,2 ) );
        ms[i] = (s.size() > 8) ? stoi( s.substr(9) ) : 0;
        // _old_version: 原来算错了 UpdatgeMillisec，直接相加了
        // 所以需要除以 1000
        ex[i] = hh[i] *3600 + mm[i] *60 + ss[i] + ms[i]/1000.0;
        if ( hh[i] >= 18 ) ex[i] -= 86400;

        s = string( TimeStamp[i] );
        if ( s.size() <= 8 ) continue; // 防止 TimeStamp 是空的
        k = stoi( s.substr( 9,2 ) );
        ts[i] = k *3600 + 
                stoi( s.substr( 12,2 ) ) *60 + 
                stoi( s.substr( 15,2 ) ) + 
                stod( s.substr( 18 ) ) / 1000000.0;
        if ( k >= 18 ) ts[i] -= 86400;

    }
}
// -----------------------------------------------------------------------------


// -----------------------------------------------------------------------------
/*
- rcpp_tick_data_cal_delta:
计算 delta 数据
*/
// [[Rcpp::export]]
void rcpp_tick_data_cal_delta(DataFrame dt) {
    const int NROW = dt.nrow();

    StringVector id = dt["InstrumentID"];
    NumericVector
        volume = dt["Volume"],
        turnover = dt["Turnover"],
        openinterest = dt["OpenInterest"],

        delta_volume = dt["DeltaVolume"],
        delta_turnover = dt["DeltaTurnover"],
        delta_openinterest = dt["DeltaOpenInterest"];

    for ( int i=0; i<NROW; ++i ) {
        // 1. 如果是第1个, 或者是新的合约代码, 则开始为 集合竞价的数据
        // 2. 否则需要减去上面的, 用来计算 delta
        if ( i == 0 || id[i] != id[i-1] ) {
            delta_volume[i] = volume[i];
            delta_turnover[i] = turnover[i];
            delta_openinterest[i] = openinterest[i];
        } else {
            delta_volume[i] = volume[i] - volume[i-1];
            delta_turnover[i] = turnover[i] - turnover[i-1];
            delta_openinterest[i] = openinterest[i] - openinterest[i-1];
        }
    }
}
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
/*
- rcpp_updateCZCEInstrumentID
计算 delta 数据
*/
// [[Rcpp::export]]
StringVector rcpp_updateCZCEInstrumentID(StringVector tradingday, StringVector oldInstrumentID) {
    const int SIZE = oldInstrumentID.size();

    StringVector newInstrumentID(SIZE);
    int tradingyear = 0, tmp_decade = 0;
    string s;

    for ( int i=0; i<SIZE; ++i ) {
        tradingyear = stoi( string(tradingday[i]).substr( 0,4 ) );

        s = string( oldInstrumentID[i] );
        if ( s.size() > 5 ) {
            newInstrumentID[i] = s;
            continue;
        }

        tmp_decade = ( tradingyear - 2000 ) /10;
        // 如果是超过 5年， 且是 0 开头，那应该就是第二个十年的开始了
        if ( s.substr(2,1) == "0" && tradingyear %10 > 5 )
            tmp_decade +=1;

        newInstrumentID[i] = s.substr( 0,2 ) + to_string( tmp_decade ) + s.substr( 2 );
    }

    return newInstrumentID;
}

// [[Rcpp::export]]
StringVector rcpp_simplyCZCEInstrumentID(StringVector newInstrumentID) {
    const int SIZE = newInstrumentID.size();
    StringVector oldInstrumentID( SIZE );

    string s;
    for ( int i=0; i<SIZE; ++i ) {
        s = string( newInstrumentID[i] );
        // Rcout << s;
        oldInstrumentID[i] = (s.size() == 5)? s : (s.substr(0,2) + s.substr(3));
    }

    return oldInstrumentID;
}
// -----------------------------------------------------------------------------


// -----------------------------------------------------------------------------
/*
- rcpp_mbar_from_tick
计算 delta 数据
*/
// [[Rcpp::export]]
DataFrame rcpp_mbar_from_tick( DataFrame &tick )
{
    const int NROW = tick.nrow();

    StringVector 
        tradingdayVector = tick["TradingDay"],
        minuteVector = tick["Minute"],
        instrumentVector = tick["InstrumentID"];
    NumericVector 
        last  = tick["LastPrice"],
        delta_turnover = tick["DeltaTurnover"],
        delta_volume = tick["DeltaVolume"],
        oi = tick["OpenInterest"],
        ex_time = tick["NumericExchTime"],
        upper_limit = tick["UpperLimitPrice"],
        lower_limit = tick["LowerLimitPrice"];

    vector<string> _mVector, m_Vector, tdVector, idVector;
    vector<double> m_open, m_high, m_low, m_close, m_ex_time, m_upper, m_lower;
    vector<double> m_volume, m_open_oi, m_high_oi, m_low_oi, m_close_oi;
    vector<double> m_turnover;

    string id;
    int last_index = 0;
    string minute;

    for ( int i = 0; i < NROW; i++ ) {
        if ( delta_volume[i] <= 0 ) continue;

        id = string(instrumentVector[i]);
        minute = string(minuteVector[i]);

        if ( std::find( idVector.begin(), idVector.end(), id ) == idVector.end() ) 
        {
            // 不在，是全新的数据
            _mVector.clear();
        }

        if ( std::find(_mVector.begin(), _mVector.end(), minute) == _mVector.end() ) 
        {
            // 不在，是新的
            _mVector.emplace_back(minute);
            
            tdVector.emplace_back( string(tradingdayVector[i]) );
            idVector.emplace_back(id);
            m_Vector.emplace_back(minute);
            
            m_open.emplace_back( last[i] );
            m_high.emplace_back( last[i] );
            m_low.emplace_back( last[i] );
            m_close.emplace_back( last[i] );
            m_volume.emplace_back( delta_volume[i] );
            m_turnover.emplace_back( delta_turnover[i] );
            m_open_oi.emplace_back(oi[i]);
            m_high_oi.emplace_back(oi[i]);
            m_low_oi.emplace_back(oi[i]);
            m_close_oi.emplace_back(oi[i]);

            m_ex_time.emplace_back(ex_time[i]);
            m_upper.emplace_back(upper_limit[i]);
            m_lower.emplace_back(lower_limit[i]);

            ++last_index;
        } else {
            // 在，是旧的
            
            m_high[last_index-1] = max( m_high[last_index-1], last[i] );
            m_low[last_index-1] = min( m_low[last_index-1], last[i] );
            m_close[last_index-1] = last[i];

            m_volume[last_index-1] += delta_volume[i];
            m_turnover[last_index-1] += delta_turnover[i];

            m_high_oi[last_index-1] = max( m_high_oi[last_index-1], oi[i] );
            m_low_oi[last_index-1] = min( m_low_oi[last_index-1], oi[i] );
            m_close_oi[last_index-1] = oi[i];
        }
    }

    // Rprintf( "index = %d\n", index );

    Rcpp::DataFrame mbar = Rcpp::DataFrame::create(
        Rcpp::Named("TradingDay") = StringVector( tdVector.begin (), tdVector.end() ),
        Rcpp::Named("InstrumentID") = StringVector( idVector.begin (), idVector.end() ),
        Rcpp::Named("Minute") = StringVector( m_Vector.begin(), m_Vector.end() ),
        Rcpp::Named("NumericExchTime") = NumericVector( m_ex_time.begin(), m_ex_time.end() ),

        Rcpp::Named("OpenPrice") = NumericVector( m_open.begin(), m_open.end() ),
        Rcpp::Named("HighPrice") = NumericVector( m_high.begin(), m_high.end() ),
        Rcpp::Named("LowPrice") = NumericVector( m_low.begin(), m_low.end() ),
        Rcpp::Named("ClosePrice") = NumericVector( m_close.begin(), m_close.end() ),

        Rcpp::Named("Volume") = NumericVector( m_volume.begin(), m_volume.end() ),
        Rcpp::Named("Turnover") = NumericVector( m_turnover.begin(), m_turnover.end() ),

        Rcpp::Named("OpenOpenInterest") = NumericVector( m_open_oi.begin(), m_open_oi.end() ),
        Rcpp::Named("HighOpenInterest") = NumericVector( m_high_oi.begin(), m_high_oi.end() ),
        Rcpp::Named("LowOpenInterest") = NumericVector( m_low_oi.begin(), m_low_oi.end() ),
        Rcpp::Named("CloseOpenInterest") = NumericVector( m_close_oi.begin(), m_close_oi.end() ),

        Rcpp::Named("UpperLimitPrice") = NumericVector( m_upper.begin(), m_upper.end() ),
        Rcpp::Named("LowerLimitPrice") = NumericVector( m_lower.begin(), m_lower.end() ),
        
        Rcpp::Named("stringsAsFactors") = false
        );

    return mbar;
}
// -----------------------------------------------------------------------------



// -----------------------------------------------------------------------------
/*
- rcpp_dbar_from_tick
计算 delta 数据
*/
// [[Rcpp::export]]
DataFrame rcpp_dbar_from_tick( DataFrame &tick )
{
    const int NROW = tick.nrow();

    StringVector 
        tradingdayVector = tick["TradingDay"],
        instrumentVector = tick["InstrumentID"];
    NumericVector 
        open  = tick["OpenPrice"],
        high  = tick["HighestPrice"],
        low   = tick["LowestPrice"],
        close = tick["ClosePrice"],
        last  = tick["LastPrice"],
        volume = tick["Volume"],
        turnover = tick["Turnover"],
        delta_volume = tick["DeltaVolume"],
        oi = tick["OpenInterest"],
        upper_limit = tick["UpperLimitPrice"],
        lower_limit = tick["LowerLimitPrice"];

    vector<string> tdVector, idVector;
    vector<double> d_open, d_high, d_low, d_close, d_upper, d_lower;
    vector<double> d_volume, d_open_oi, d_high_oi, d_low_oi, d_close_oi;
    vector<double> d_turnover;

    string id;
    int last_index = 0;

    for ( int i = 0; i < NROW; i++ ) {
        if ( delta_volume[i] <= 0 ) continue;

        id = string(instrumentVector[i]);

        if ( std::find(idVector.begin(), idVector.end(), id) == idVector.end() ) {
            // 不在，是新的
            
            tdVector.emplace_back( string(tradingdayVector[i]) );
            idVector.emplace_back(id);
            
            d_open.emplace_back( ( NumericVector::is_na(open[i]) || open[i] == 0 )? last[i] : open[i] );
            d_high.emplace_back( ( NumericVector::is_na(high[i]) || high[i] == 0 )? last[i] : high[i] );
            d_low.emplace_back( ( NumericVector::is_na(low[i]) || open[i] == 0 )? last[i] : low[i] );
            d_close.emplace_back( ( NumericVector::is_na(close[i]) || close[i] == 0)? last[i] : close[i] );
            d_volume.emplace_back( volume[i] );
            d_turnover.emplace_back( turnover[i] );
            d_open_oi.emplace_back(oi[i]);
            d_high_oi.emplace_back(oi[i]);
            d_low_oi.emplace_back(oi[i]);
            d_close_oi.emplace_back(oi[i]);

            d_upper.emplace_back(upper_limit[i]);
            d_lower.emplace_back(lower_limit[i]);

            ++last_index;
        } else {
            // 在，是旧的
            d_high[last_index-1] = max( d_high[last_index-1], ( NumericVector::is_na(high[i]) || high[i] == 0 )? last[i] : high[i] );
            d_low[last_index-1] = min( d_low[last_index-1], ( NumericVector::is_na(low[i]) || low[i] == 0 )? last[i] : low[i] );
            d_close[last_index-1] = (NumericVector::is_na(close[i]) || close[i] == 0)? last[i] : close[i];

            d_volume[last_index-1] = volume[i];
            d_turnover[last_index-1] = turnover[i];

            d_high_oi[last_index-1] = max( d_high_oi[last_index-1], oi[i] );
            d_low_oi[last_index-1] = min( d_low_oi[last_index-1], oi[i] );
            d_close_oi[last_index-1] = oi[i];
        }
    }

    // Rprintf( "index = %d\n", index );

    Rcpp::DataFrame dbar = Rcpp::DataFrame::create(
        Rcpp::Named("TradingDay") = StringVector( tdVector.begin (), tdVector.end() ),
        Rcpp::Named("InstrumentID") = StringVector( idVector.begin (), idVector.end() ),
        
        Rcpp::Named("UpperLimitPrice") = NumericVector( d_upper.begin(), d_upper.end() ),
        Rcpp::Named("LowerLimitPrice") = NumericVector( d_lower.begin(), d_lower.end() ),

        Rcpp::Named("OpenPrice") = NumericVector( d_open.begin(), d_open.end() ),
        Rcpp::Named("HighPrice") = NumericVector( d_high.begin(), d_high.end() ),
        Rcpp::Named("LowPrice") = NumericVector( d_low.begin(), d_low.end() ),
        Rcpp::Named("ClosePrice") = NumericVector( d_close.begin(), d_close.end() ),
        Rcpp::Named("Volume") = NumericVector( d_volume.begin(), d_volume.end() ),
        Rcpp::Named("Turnover") = NumericVector( d_turnover.begin(), d_turnover.end() ),
        Rcpp::Named("OpenPosition") = NumericVector( d_open_oi.begin(), d_open_oi.end() ),
        Rcpp::Named("HighPosition") = NumericVector( d_high_oi.begin(), d_high_oi.end() ),
        Rcpp::Named("LowPosition") = NumericVector( d_low_oi.begin(), d_low_oi.end() ),
        Rcpp::Named("ClosePosition") = NumericVector( d_close_oi.begin(), d_close_oi.end() ),
        Rcpp::Named("stringsAsFactors") = false
        );

    return dbar;
}
// -----------------------------------------------------------------------------
