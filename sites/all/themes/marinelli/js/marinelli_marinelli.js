/**
 * common theme behaviours
 * 
 */
 
jQuery(document).ready(function($){
	$("*").removeClass("marinelli-hide-no-js"); // remove the hide class (see common.css)
	
	//By default hide the ship pane
	$("#quotes-pane").hide();
	$(".field-delivery_ucxf_tke_member," +
			".field-delivery_company, " +
			".field-delivery_street1," +
			".field-delivery_street2, " +
			".field-delivery_city, " +
			".field-delivery_zone, " +
			".field-delivery_country," +
			".field-delivery_postal_code," +
			".field-delivery_phone").hide();	

	
	
	$("#edit-panes-delivery-address-delivery-ucxf-delivery").change(function(){

		//alert("Here goes calling");
		valueFor = $(this).val().toLowerCase();
		$("#uc-cart-checkout-form input[type='text']").val("");
		
		$(".field-delivery_ucxf_tke_member," +
				".field-delivery_ucxf_sorority_house," +
				".field-delivery_company, " +
				".field-delivery_street1," +
				".field-delivery_street2, " +
				".field-delivery_city, " +
				".field-delivery_zone, " +
				".field-delivery_country," +
				".field-delivery_postal_code," +
				".field-delivery_phone").hide();
		
		switch(valueFor)
		{
			case "tke":
				$(".field-delivery_ucxf_tke_member").show();
				$("#quotes-pane").hide();
			  break;
			case "sorority":
				$(".field-delivery_ucxf_sorority_house").show();
				$("#quotes-pane").hide();
			  break;
			case "ship":
				$(".field-delivery_company, " +
						".field-delivery_street1," +
						".field-delivery_street2, " +
						".field-delivery_city, " +
						".field-delivery_zone, " +
						".field-delivery_country," +
						".field-delivery_postal_code," +
						".field-delivery_phone").show();
				$("#quotes-pane").show();
			  break;
			default:
				console.log("execute code block default");
		}
		
		$.get("http://drupal.cs/changeshipping/"+valueFor,function(data){
			
//			if(valueFor == 'ship')
//			{	
//			  window.location.href = "/cart/checkout";
//			}	
			  
		});
	});
	
	
	/*
	 * Validation logic should be placed
	 */
	$("#uc-cart-checkout-form").validate({
		rules: {
			"panes[delivery][address][delivery_first_name]": "required",
			"panes[delivery][address][delivery_last_name]": "required",
			"panes[delivery][address][delivery_ucxf_sorority_house]": "required",
			"panes[delivery][address][delivery_ucxf_tke_member]": "required",
			"panes[delivery][address][delivery_company]": "required",
			"panes[delivery][address][delivery_street1]": "required",
			"panes[delivery][address][delivery_city]": "required",
			"panes[delivery][address][delivery_zone]": "required",
			"panes[delivery][address][delivery_country]": "required",
			"panes[delivery][address][delivery_postal_code]": "required"
			
		},
		messages: {
			"panes[delivery][address][delivery_first_name]": "Please enter your First Name",
			"panes[delivery][address][delivery_last_name]": "Please enter your Last Name",
			"panes[delivery][address][delivery_ucxf_sorority_house]": "Please enter your Sorority House",
			"panes[delivery][address][delivery_ucxf_tke_member]": "Please enter your Tke Member",
			"panes[delivery][address][delivery_company]": "Please enter your Company",
			"panes[delivery][address][delivery_street1]": "Please enter your Street Address",
			"panes[delivery][address][delivery_city]": "Please enter your City",
			"panes[delivery][address][delivery_zone]": "Please enter your State/Province",
			"panes[delivery][address][delivery_country]": "Please enter your Country",
			"panes[delivery][address][delivery_postal_code]": "Please enter your Postal Code"
		}
	});
	
	/*
	 * Avoid the validation on cancel button
	 */
	$("#edit-cancel").addClass("cancel");
	
});