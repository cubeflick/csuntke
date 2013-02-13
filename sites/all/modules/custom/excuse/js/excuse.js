(function($) {

	$(".delete-excuse input[type=submit]").click(function(){
		var isTrue = confirm("Are you sure to delete the excuse");
		if (isTrue==true)
		  {
			return true;
		  }
		else
		  {
			return false;
		  }		
		
	})
})(jQuery);