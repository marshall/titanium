App.UI.registerUIComponent('control','yui_paginator',
{
	create: function()
	{
		this.options = null;
		this.element = null;
		this.getAttributes = function()
		{
			return [
			{name: 'message', optional: true, 
			 description: "Message to be sent on page request click"},
			{name: 'rowsPerPage', optional: true, 
			 description: "size of each page",defaultValue:20},
			{name: 'totalRecords', optional: true, 
			 description: "total number of records",defaultValue:100},
			{name: 'firstPageLinkLabel', optional: true, 
			 description: "label for first page"},
			{name: 'lastPageLinkLabel', optional: true, 
			 description: "label for last page"},
			{name: 'previousPageLinkLabel', optional: true, 
			 description: "label for previous page"},
			{name: 'nextPageLinkLabel', optional: true, 
			 description: "label for next page"},
			{name: 'pageLinks', optional: true, defaultValue:5,
			 description: "number of page links to display"},
			{name: 'initialPage', optional: true, defaultValue:1,
			 description: "initial page to display"},
			{name: 'pageReportTemplate', optional: true, defaultValue:'Page {currentPage} of {totalPages}',
			 description: "template for show page x of y"},
			
			{name: 'alwaysVisible', optional: true,defaultValue:false, 
			 description: "show page controls always even if no data"}

		];
		};
		
		this.getVersion = function()
		{
			// leave this as-is and only configure from the build.yml file 
			// and this will automatically get replaced on build of your distro
			return '__VERSION__';
		};
		
		this.getSpecVersion = function()
		{
			return 1.0;
		};

		this.getActions = function()
		{
		};

		/*
		*	this.getControlJS will pass an array of files to a dyanmic loader. 
		*/
		this.getControlJS = function()
		{
			return ['../../../common/js/yahoo-min.js','js/paginator-min.js']
		}
				
		/*
		*	this.getControlCSS will pass an array of files to a dyanmic loader. 
		*/		
		this.getControlCSS = function()
		{
			return ['assets/skins/sam/paginator-skin.css']
		}
		this.rowsPerPage = function(value)
		{
			this.paginator.setRowsPerPage(parseInt(App.getActionValue(value,'value',this.options.rowsPerPage)))
			this.paginator.render();
		}  
		this.page = function(value)
		{
			this.paginator.setPage(parseInt(App.getActionValue(value,'value',this.options.initialPage)))
			this.paginator.render();
		}  
		this.totalRecords = function(value)
		{
			this.paginator.setTotalRecords(parseInt(App.getActionValue(value,'value',this.options.totalRecords)))
			this.paginator.render();
		}  
		this.update = function(value)
		{
			this.paginator.setRowsPerPage(parseInt(App.getActionValue(value,'rowsPerPage',this.options.rowsPerPage)))
			this.paginator.setPage(parseInt(App.getActionValue(value,'page',this.options.initialPage)))
			this.paginator.setTotalRecords(parseInt(App.getActionValue(value,'totalRecords',this.options.totalRecords)))
			this.paginator.render();
			
		}
		this.getActions = function()
		{
			return ['rowsPerPage','page','totalRecords','update'];
		} 
		/**
		 * This is called when the control is loaded and applied for a specific element that 
		 * references (or uses implicitly) the control.
		 */
		this.build = function(element,options)
		{
			this.element=element;
			this.options = options;
			var paginator = new YAHOO.widget.Paginator({ 
				    rowsPerPage  : parseInt(options.rowsPerPage), 
				    totalRecords : parseInt(options.totalRecords), 
				    containers   : element.id, 
				    template     : element.innerHTML,
					alwaysVisible: options.alwaysVisible,
					nextPageLinkLabel:options.nextPageLinkLabel,
					previousPageLinkLabel:options.previousPageLinkLabel,
					firstPageLinkLabel:options.firstPageLinkLabel,
					lastPageLinkLabel:options.lastPageLinkLabel,
					initialPage:parseInt(options.initialPage),
					pageReportTemplate    : options.pageReportTemplate,
					pageLinks:parseInt(options.pageLinks)
			});
			paginator.subscribe('changeRequest',function(state)
			{
				if (options.message != null)
				{
					$MQ(options.message,state);
				}
				paginator.setState(state)
			})
			paginator.render();
			this.paginator = paginator;

		};
	}
});
