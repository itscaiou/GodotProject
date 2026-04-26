extends Control


func ready():
	$AudioStreamPlayer2D.play()

func _on_button_pressed() -> void:
	get_tree().change_scene_to_file("res://Cenas/prologo.tscn")
	$AudioStreamPlayer2D.stop()

func _on_button_2_pressed() -> void:
	get_tree().quit()
	$AudioStreamPlayer2D.stop()
