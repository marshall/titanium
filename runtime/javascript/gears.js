
// gearsFactory is defined inside a function block
// and can safely be exposed inside titanium but will
// not be available outside of the titanium.js (i.e. app)
var gearsFactory = document.createElement("object");
gearsFactory.style.display = "none";
gearsFactory.width = 0;
gearsFactory.height = 0;
gearsFactory.type = "application/x-gears-titanium";
document.documentElement.appendChild(gearsFactory);

if (!gearsFactory.create)
{
	//oops! Gears not loading
	ti.App.debug("Gears plugin didn't load and is unavailable!");
	gearsFactory  = null;
}
else
{
	ti.App.debug("Gears plugin loaded. Awesome!");
}
