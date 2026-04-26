extends Area2D

const SPEED := 150

var direction := 1
var damage := 2
var effect_type := ""
var effect_duration := 0.0
var owner_node = null

func _ready():
	print("Projétil criado:", name)
	$AudioStreamPlayer2D.play()

func setup(dir: int, dmg: int, effect: String = "", duration: float = 0.0, owner_ref = null):
	direction = dir
	damage = dmg
	effect_type = effect
	effect_duration = duration
	owner_node = owner_ref
	
	$AnimatedSprite2D.flip_h = direction < 0

func set_direction(dir):
	direction = dir
	$AnimatedSprite2D.flip_h = direction < 0

func _physics_process(delta: float) -> void:
	position.x += SPEED * direction * delta

func _on_visible_on_screen_notifier_2d_screen_exited() -> void:
	queue_free()

func _on_area_entered(area: Area2D) -> void:
	print("Encostou na área:", area.name)

	if area.name == "Hurtbox":
		var target = area.get_parent()
		
		if target == owner_node:
			return
		
		if target.has_method("take_hit"):
			target.take_hit(damage, effect_type, effect_duration)
		elif target.has_method("take_damage"):
			target.take_damage(damage)
		
		queue_free()
