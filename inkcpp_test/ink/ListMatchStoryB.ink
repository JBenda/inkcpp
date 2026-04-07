LIST Indoor = Kitchen, Floor, Livingroom
LIST Outdoor = Street, Balcony, Garden
VAR current_stage = ()
~ current_stage = Floor

You are currently at {current_stage}
* More
- You are still at {current_stage} - all posibilities are {LIST_ALL(current_stage)}
-> END
