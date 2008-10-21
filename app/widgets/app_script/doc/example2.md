Title: Accessing Properties inside an app:script

You can access the properties of a data payload inside an `app:script`:

++example
<button on="click then l:hey.there[text='Hi There',name=Steve]">Click Me!</button>
<app:script on="l:hey.there then execute">
    alert(this.data.text + ', ' + this.data.name);
</app:script>
--example