#include <iostream>
#include <cassert>
#include <algorithm>
#include <cmath>
using namespace std;

/*
 * Our test functions to test the methods to check if we obtain the correct result
 * */
void testGetFreeIntervals();
void testWithNoBookedCalendar();
void testWithOnlyOneCalendarBooked();
void testMaxOnTime();
void testMinOnTime();
void testTimeDifference();


/*
 * The Time class in which we will store the time as hour:minute
 * */
class Time {
public:
    int hour;
    int minute;

    Time(int hour, int minute) : hour{hour}, minute{minute} {

    }

    bool operator<(const Time &time) const {
        if (hour == time.hour) {
            return minute < time.minute;
        }
        return hour < time.hour;
    }

    bool operator>=(const Time &time) const {
        if (hour == time.hour) {
            return minute >= time.minute;
        }
        return hour >= time.hour;
    }

    bool operator==(const Time &time) const {
        return hour == time.hour && minute == time.minute;
    }

    friend ostream &operator<<(ostream &os, const Time &time) {
        os << time.hour << ":" << time.minute;
        return os;
    };

    int operator-(const Time& time){
        int result = (hour * 60 + minute) - (time.hour * 60 + time.minute);
        return abs(result);
    }
};


/*
 * The interval class in which we will store the start and the stop of a not available period
 */
class Interval{
public:
    Time start;
    Time stop;
    Interval(Time start, Time stop): start{start}, stop{stop}{

    }

    bool operator==(const Interval &interval) const {
        return start == interval.start && stop == interval.stop;
    }

    friend ostream& operator<<(ostream& os, const Interval& interval){
        os << interval.start << " - " << interval.stop;
        return os;
    }
};


/*
 * Calendar class in which we will store the not available periods during the day and also the starting day Time and the
 * finishing day Time
 * */
class Calendar{
public:
    vector<Interval> notAvailableIn;
    Time startingDayTime;
    Time finishingDayTime;

    Calendar(vector<Interval> notAvailableIn, Time startingDayTime, Time finishingDayTime):
    notAvailableIn{notAvailableIn},
    startingDayTime{startingDayTime},
    finishingDayTime{finishingDayTime}
    {

    }
};
/*
 * This function computes the maximum between two time classes and return the biggest one (the latest)
 * max(10:30, 10:40) => 10:40
 * The formula transform into minutes and compare those two amount of times
 * Alternatively I could use an if-else approach (comparing hours, return greatest, otherwise compare minutes)
*/
 Time max(Time time1, Time time2){
    return (time1.hour * 60 + time1.minute) >= (time2.hour * 60 + time2.minute) ? time1 : time2;
}

/*
 * This function computes the minimum between two time classes and return the smallest one(the earliest)
 * min(10:30,10:40) => 10:30
 * same as for max but for min
*/
Time min(Time time1, Time time2){
    return (time1.hour * 60 + time1.minute) >= (time2.hour * 60 + time2.minute) ? time2 : time1;
}

vector<Interval> getFreeIntervals(Calendar firstCalendar, Calendar secondCalendar, int minimumFreeTime){
    /*
     * I also assumed that input data is correct and no validation is required.
     * This methods get free intervals from given two calendar which are greatest or equal than the minimumFreeTime
     * I consider that the given intervals in input is already sorted and there are no intervals intersection
     * for an employee calendar. Eg: [[10:30, 11:00], [11:00,12:00]] - correct
     *                                [[10:30,11:00], [10:45,12:00]] - wrong the first interval overlap with the other one
     * I merge all the intervals into one array, by their starting time, using merge sort algorithm having
     * an O(N+M) worst case complexity and O(N+M) space complexity, then merge the overlapping intervals
     * into one single interval which contains the starting time the minimum and the ending time the maximum
     * and store it into an array. Then we can extract the free time by comparing the first element of previous array
     * with the startingDayTime to check if there is a slot between the latest starting day time and the first busy time.
     * Then we compare the intervals with the previous one by checking the previous interval end time with current
     * interval starting time and check if that gap is greater than minimumFreeTime. At the final we also check
     * if there exists a gap between last busy interval and the finishingDayTime.
     * Worst case complexity: O(N+M) where N = first calendar size and M = second calendar size
     * Total space complexity: O(N+M)
     * */
    /*
     * One observation we can do is that our answer sits between the latest starting day time
     * and the earliest leaving day time.
     * */
    Time startingTime = max(firstCalendar.startingDayTime,secondCalendar.startingDayTime);
    Time finishingTime = min(firstCalendar.finishingDayTime,secondCalendar.finishingDayTime);
    vector<Interval> freeIntervals;
    vector<Interval> mergedIntervals;
    vector<Interval> allIntervals;
    int firstIntervalIndex, secondIntervalIndex;
    firstIntervalIndex = secondIntervalIndex = 0;

    /*
     * We merge the intervals by their starting time using the merge sort algorithm between two arrays(of Intervals)
     * */
    while(firstIntervalIndex < firstCalendar.notAvailableIn.size() &&
    secondIntervalIndex < secondCalendar.notAvailableIn.size()){
        if(firstCalendar.notAvailableIn[firstIntervalIndex].start < secondCalendar.notAvailableIn[secondIntervalIndex].start){
            allIntervals.push_back(firstCalendar.notAvailableIn[firstIntervalIndex]);
            firstIntervalIndex++;
        }
        else if(secondCalendar.notAvailableIn[secondIntervalIndex].start < firstCalendar.notAvailableIn[firstIntervalIndex].start){
            allIntervals.push_back(secondCalendar.notAvailableIn[secondIntervalIndex]);
            secondIntervalIndex++;
        }
        else {
            allIntervals.push_back(secondCalendar.notAvailableIn[secondIntervalIndex]);
            allIntervals.push_back(firstCalendar.notAvailableIn[firstIntervalIndex]);
            firstIntervalIndex++;
            secondIntervalIndex++;
        }
    }
    /*
     * Check if there are no elements which has not been added to allIntervals vector
     * */
    while(firstIntervalIndex < firstCalendar.notAvailableIn.size()){
        allIntervals.push_back(firstCalendar.notAvailableIn[firstIntervalIndex]);
        firstIntervalIndex++;
    }

    while(secondIntervalIndex < secondCalendar.notAvailableIn.size()){
        allIntervals.push_back(secondCalendar.notAvailableIn[secondIntervalIndex]);
        secondIntervalIndex++;
    }

    /*
     * Considering the edge case when the employees have 0 meetings (i.e they are free all day)
     * In this case the result is the maximum of the latest starting day times and minimum of ending day times
     * */
    if(allIntervals.size() == 0){
        if(finishingTime - startingTime < minimumFreeTime){
            return freeIntervals;
        }
        freeIntervals.push_back(Interval{
            startingTime,finishingTime
        });
        return freeIntervals;
    }

    mergedIntervals.push_back(allIntervals[0]);

    /*
     * We merge the intervals which intersects because we want to get the largest interval in which we cannot get any
     * free time
     * */
    for (int index = 1; index < allIntervals.size(); ++index) {
            Interval lastInterval = mergedIntervals[mergedIntervals.size() - 1];
            if(lastInterval.stop  >= allIntervals[index].start){
                lastInterval.start  = min(lastInterval.start,allIntervals[index].start);
                lastInterval.stop = max(lastInterval.stop,allIntervals[index].stop);
                mergedIntervals[mergedIntervals.size() - 1] = lastInterval;
            }
            else{
                mergedIntervals.push_back(allIntervals[index]);
            }
    }

        /*
         * We compute the gaps between latest starting day time, between each interval and the finishing day time
         * and check if the gap is greater or equal to the minimum free time and save that interval is it respect
         * the condition by saving the previous interval finishing time and current interval starting time.
         * */
    for (int index = 0; index < mergedIntervals.size(); ++index) {
        //check for day start time and first busy interval
        if(index == 0 &&
         startingTime < mergedIntervals[index].start &&
         startingTime - mergedIntervals[index].start >= minimumFreeTime){
                freeIntervals.emplace_back(
                    startingTime,
                    mergedIntervals[index].start
                );
        }

        //check for current interval and previous interval
        if(index > 0 && mergedIntervals[index - 1].stop < mergedIntervals[index].start &&
           mergedIntervals[index - 1].stop - mergedIntervals[index].start >= minimumFreeTime){
            freeIntervals.emplace_back(mergedIntervals[index - 1].stop , mergedIntervals[index].start);
        }

        //check for last interval and day finish time
        if(index == mergedIntervals.size() - 1 &&
        mergedIntervals[index].stop < finishingTime &&
        (finishingTime - mergedIntervals[index].stop) >= minimumFreeTime){
            freeIntervals.emplace_back(
                    mergedIntervals[index].stop,
                    finishingTime
            );
        }
    }

    return freeIntervals;
}

int main() {
    cout<<"Running all tests:\n";
    testGetFreeIntervals();
    testWithNoBookedCalendar();
    testWithOnlyOneCalendarBooked();
    testMaxOnTime();
    testMinOnTime();
    testTimeDifference();
    cout<<"All tests passed\n";
    return 0;
}

void testGetFreeIntervals(){

   vector<Interval> interval1{
       Interval{
           Time{
               9,0
           },
           Time{
               10,30
           }
       },
       Interval{
               Time{
                       12,0
               },
               Time{
                       13,0
               }
       },
       Interval{
               Time{
                       16,0
               },
               Time{
                       18,0
               }
       },
   };

    vector<Interval> interval2{
            Interval{
                    Time{
                            10,0
                    },
                    Time{
                            11,30
                    }
            },
            Interval{
                    Time{
                            12,30
                    },
                    Time{
                            14,30
                    }
            },
            Interval{
                    Time{
                            14,30
                    },
                    Time{
                            15,0
                    }
            },
            Interval{
                    Time{
                            16,0
                    },
                    Time{
                            17,0
                    }
            }
    };

    Calendar calendar1{interval1, Time{9,0},Time{20,0}};
    Calendar calendar2{interval2,Time{10,0},Time{18,30}};
    int minimumFreeTime = 30;
    vector<Interval> intervals = getFreeIntervals(calendar1,calendar2, minimumFreeTime);

    for (int i = 0; i < intervals.size(); ++i) {
        cout<<intervals[i]<<"\n";
    }

    Interval answer1 {
        Time{
            11,30
        },
        Time{12,0}
    };

    Interval answer2 {
        Time{
            15,0
        },
        Time{
            16,0
        }
    };

    Interval answer3{
        Time{
            18,0
        },
        Time{
            18,30
        }
    };

    assert(intervals[0] == answer1);
    assert(intervals[1] == answer2);
    assert(intervals[2] == answer3);
    assert(intervals.size() == 3);


    intervals = getFreeIntervals(calendar2,calendar1, minimumFreeTime);
    assert(intervals[0] == answer1);
    assert(intervals[1] == answer2);
    assert(intervals[2] == answer3);
    assert(intervals.size() == 3);
}

void testWithNoBookedCalendar(){
    vector<Interval> interval1;
    vector<Interval> interval2;

    Calendar calendar1{interval1,Time{9,0},Time{18,0}};
    Calendar calendar2{interval1,Time{8,0},Time{17,25}};

    int minimumFreeTime = 30;
    vector<Interval> intervals = getFreeIntervals(calendar1,calendar2, minimumFreeTime);

    for (int i = 0; i < intervals.size(); ++i) {
        cout<<intervals[i]<<"\n";
    }

    Interval answer {
        Time{
            9,0
        },
        Time{
            17,25
        }
    };

    assert(intervals.size() == 1);
    assert(intervals[0] == answer);

}

void testWithOnlyOneCalendarBooked(){
    vector<Interval> interval1{
            Interval{
                    Time{
                            9,0
                    },
                    Time{
                            12,30
                    }
            },
            Interval{
                    Time{
                            12,0
                    },
                    Time{
                            13,0
                    }
            },
            Interval{
                    Time{
                            13,0
                    },
                    Time{
                            16,15
                    }
            },
    };

    vector<Interval> interval2;

    Calendar calendar1{interval1, Time{7,30},Time{16,30}};
    Calendar calendar2{interval2,Time{8,0},Time{18,0}};
    int minimumFreeTime = 30;

    vector<Interval> intervals = getFreeIntervals(calendar1,calendar2, minimumFreeTime);

    for (int i = 0; i < intervals.size(); ++i) {
        cout<<intervals[i]<<"\n";
    }

    Interval answer1{
            Time{
                    8,0
            },
            Time{
                    9,0
            }
    };

    assert(intervals.size() == 1);
    assert(intervals[0] == answer1);

    intervals = getFreeIntervals(calendar2,calendar1, minimumFreeTime);

    assert(intervals.size() == 1);
    assert(intervals[0] == answer1);
}

void testMaxOnTime(){
    Time t1{10,30};
    Time t2{10,40};
    Time answer = max(t2,t1);
    assert(answer.hour == 10 && answer.minute == 40);
}

void testMinOnTime(){
    Time t1{10,30};
    Time t2{10,40};
    Time answer = min(t2,t1);
    assert(answer.hour == 10 && answer.minute == 30);
}

void testTimeDifference(){
    Time t1{10,30};
    Time t2{10,40};
    int result = t1 - t2;
    assert(result == 10);
}