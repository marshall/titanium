Title: Simple Example

This is a simple example that uses the `<app:statemachine>`.
	
Here's an example of a State Machine using my personal defense condition (MYDEFCON).  Click the links below to generate changes to MYDEFCON state.

<style>

.defcon2 
{ 
   margin:0px auto; 
   padding: 5px; 
   border-style: solid; 
   border-width: 1px; 
   border-color: #F0C000; 
   background-color: #FFFFCE; 
}

.defcon1 
{ 
   margin:0px auto; 
   padding: 5px; 
   border-style: solid; 
   border-width: 1px; 
   border-color: #c00; 
   background-color: #fcc; 
}

.defcon3 
{ 
   margin:0px auto; 
   padding: 5px; 
   border-style: solid; 
   border-width: 1px; 
   border-color: #3c78b5; 
   background-color: #D8E4F1; 
}

.defcon4 
{ 
   margin:0px auto; 
   padding: 5px; 
   border-style: solid; 
   border-width: 1px; 
   border-color: #090; 
   background-color: #dfd; 
}       
.defcon5
{ 
   margin:0px auto; 
   padding: 5px; 
   border-style: solid; 
   border-width: 1px; 
   border-color: #090; 
   background-color: #dfd; 
}       

</style>

++example
<a on="click then l:mydefcon.change[value=one]">Some jerk getting chocolate in my peanut butter</a> <br />
<a on="click then l:mydefcon.change[value=two]">Stuck in Atlanta traffic</a>  <br />
<a on="click then l:mydefcon.change[value=three]">The world supply of Gruyere cheese is eliminated by angry cheese-eating locusts</a>  <br />
<a on="click then l:mydefcon.change[value=four]">I see another Jerry Bruckheimer film</a> <br />
<a on="click then l:mydefcon.change[value=five]">Free Beer</a>


<div style="margin:10px"> 
</div>

<div class="defcon1" on="mydefcon[one] then show else hide">
	MYDEFCON 1 [comments censored by legal department]
</div>
<div class="defcon2" on="mydefcon[two] then show else hide">
	MYDEFCON 2. Redrum.  Redrum.  Redrum.
</div>
<div class="defcon3" on="mydefcon[three] then show else hide">
	MYDEFCON 3. Damn. Now I'm getting angry.
</div>
<div class="defcon4" on="mydefcon[four] then show else hide">
	MYDEFCON 4. Okay that's 2 hours I'll never get back, but I'm maintaining my composure.
</div>
<div class="defcon5" on="mydefcon[five] then show else hide">
	MYDEFCON 5. Me - at peace... 
</div>

<app:statemachine id="mydefcon" initial="five">  
	<state name="one" if="l:mydefcon.change[value=one]"></state>  
	<state name="two" if="l:mydefcon.change[value=two]"></state>  
	<state name="three" if="l:mydefcon.change[value=three]"></state>  
	<state name="four" if="l:mydefcon.change[value=four]"></state>  
	<state name="five" if="l:mydefcon.change[value=five]"></state>  
</app:statemachine>
--example