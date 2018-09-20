(function($) {
  Drupal.behaviors.accordion_blocks = {
	  attach: function() {
	    $('.accordion_blocks_container').accordion({header: "h2", autoHeight: true});
	  }
  };
  setTimeout(function(){ ///Modificar el color de los headers del accordeon de redes sociales
  		var fb_like = $('.fb-like-box').parent().prev();
  		var twitter_time = $('.twitter-timeline').parent().prev();
  		$(fb_like).css('background', 'none');
  		$(fb_like).css('background-color', '#3b5998');
  		$(fb_like).children().css('color', '#fff');
  		$(twitter_time).css('background', 'none');
  		$(twitter_time).css('background-color', '#4099ff');
  		$(twitter_time).children().css('color', '#fff');
	}, 1000); //Se añade tiempo para que termine la carga del componente
})(jQuery);


