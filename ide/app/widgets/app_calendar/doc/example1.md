Title: Simple Example

This is a simple example that uses the `<app:calendar>`.
	
	<app:calendar title="Select Start Date" inputId="jobfilter_date_start" on="l:calendarexample.start then execute"
		minDate="10/16/2007">
	</app:calendar>
	<input type="text" id="jobfilter_date_start" size="10" MAXLENGTH="10" />
	<img src="calendar.png" on="click then l:calendarexample.start" />

