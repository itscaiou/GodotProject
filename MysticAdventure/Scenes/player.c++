extends CharacterBody2D

class_name Player

signal died

const DISPARO := preload("res://Poderes/disparos_magicos.tscn")
const RAJADA := preload("res://Poderes/rajada_gelada.tscn")
const FOGO := preload("res://Poderes/bola_de_fogo.tscn")
const FENOMENAL := preload("res://Poderes/poder_fenomenal.tscn")

@export_group("Locomotion")
@export var speed: float = 200.0
@export var jump_velocity: float = -450.0
@export var run_speed_damping: float = 0.5

@export_group("Health")
@export var max_health: int = 100
@export var life: int = 100

@export_group("Cooldowns")
@export var cooldown_disparos_magicos: float = 2.0
@export var cooldown_rajada_gelada: float = 10.0
@export var cooldown_bola_de_fogo: float = 12.0
@export var cooldown_poder_fenomenal: float = 25.0

var is_dead := false

var unlocked_powers = {
	"disparos_magicos": false,
	"rajada_gelada": false,
	"bola_de_fogo": false,
	"poder_fenomenal": false
}

var power_ready = {
	"disparos_magicos": true,
	"rajada_gelada": true,
	"bola_de_fogo": true,
	"poder_fenomenal": true
}

var power_cooldowns = {}

var power_cooldown_remaining = {
	"disparos_magicos": 0.0,
	"rajada_gelada": 0.0,
	"bola_de_fogo": 0.0,
	"poder_fenomenal": 0.0
}

var gravity = ProjectSettings.get_setting("physics/2d/default_gravity")
var facing_direction := 1

var is_casting := false
var pending_spell := ""

var is_frozen := false
var is_burning := false
var is_marked := false

@export var burn_damage := 1

func _ready():
	add_to_group("player")
	life = max_health
	
	power_cooldowns = {
		"disparos_magicos": cooldown_disparos_magicos,
		"rajada_gelada": cooldown_rajada_gelada,
		"bola_de_fogo": cooldown_bola_de_fogo,
		"poder_fenomenal": cooldown_poder_fenomenal
	}
	
	configure_powers_by_phase()

func lock_all_powers():
	for power_name in unlocked_powers.keys():
		unlocked_powers[power_name] = false

func configure_powers_by_phase():
	lock_all_powers()
	
	var current_scene_name = get_tree().current_scene.name
	print("Cena atual: ", current_scene_name)
	
	match current_scene_name:
		"Fase1":
			unlocked_powers["disparos_magicos"] = true
		
		"Fase2":
			unlocked_powers["disparos_magicos"] = true
			unlocked_powers["rajada_gelada"] = true
			
		"Fase 3":
			unlocked_powers["disparos_magicos"] = true
			unlocked_powers["rajada_gelada"] = true
			unlocked_powers["bola_de_fogo"] = true
			
		"FaseBoss":
			unlocked_powers["disparos_magicos"] = true
			unlocked_powers["rajada_gelada"] = true
			unlocked_powers["bola_de_fogo"] = true
			unlocked_powers["poder_fenomenal"] = true
			
	print("Poderes desbloqueados: ", unlocked_powers)

func _physics_process(delta):
	apply_gravity(delta)
	handle_jump()
	handle_movement(delta)
	update_spell_points()
	handle_spell_input()
	update_animation()
	move_and_slide()
	update_cooldowns(delta)

func apply_gravity(delta):
	if not is_on_floor():
		velocity.y += gravity * delta

func handle_jump():
	if is_casting or is_frozen:
		return
	
	if Input.is_action_just_pressed("jump") and is_on_floor():
		velocity.y = jump_velocity
	
	if Input.is_action_just_released("jump") and velocity.y < 0:
		velocity.y *= 0.5

func handle_movement(delta):
	if is_frozen:
		velocity.x = 0
		return
	
	var direction := Input.get_axis("left", "right")
	
	if direction != 0:
		velocity.x = lerp(velocity.x, speed * direction, run_speed_damping * delta)
		facing_direction = sign(direction)
	else:
		velocity.x = move_toward(velocity.x, 0, speed * delta)

func update_cooldowns(delta):
	for power_name in power_cooldown_remaining.keys():
		if power_cooldown_remaining[power_name] > 0:
			power_cooldown_remaining[power_name] -= delta
			
			if power_cooldown_remaining[power_name] <= 0:
				power_cooldown_remaining[power_name] = 0
				power_ready[power_name] = true
				print(power_name, " pronto para uso novamente")

func take_hit(amount: int, effect_type: String = "", effect_duration = 0.0):
	if is_dead:
		return
	
	var final_damage = amount
	
	if is_marked and effect_type != "mark":
		final_damage += 2
		is_marked = false
		$MarkTimer.stop()
		print("Player: marca consumida")
	
	take_damage(final_damage)
	
	match effect_type:
		"freeze":
			apply_freeze(effect_duration)
		"burn":
			apply_burn(effect_duration)
		"mark":
			apply_mark(effect_duration)

func apply_freeze(duration: float):
	if is_dead:
		return
	
	is_frozen = true
	velocity = Vector2.ZERO
	$FreezeTimer.start(duration)
	print("Player congelado por ", duration)

func apply_burn(duration: float):
	if is_dead:
		return
	
	is_burning = true
	$BurnTimer.start(duration)
	$BurnTickTimer.start(1.0)
	print("Player queimando por ", duration)

func apply_mark(duration: float):
	if is_dead:
		return
	
	is_marked = true
	$MarkTimer.start(duration)
	print("Player marcado por ", duration)

func take_damage(amount: int):
	if is_dead:
		return
	
	life -= amount
	
	if life < 0:
		life = 0
	
	print("Player tomou dano:", amount)
	print("Vida restante:", life)
	
	if life <= 0:
		die()

func die():
	if is_dead:
		return
	
	is_dead = true
	emit_signal("died")
	queue_free()

func update_spell_points():
	$Spell_point.position.x = abs($Spell_point.position.x) * facing_direction
	$Spell_point2.position.x = abs($Spell_point2.position.x) * facing_direction

func handle_spell_input():
	if is_casting or is_frozen:
		return
	
	if Input.is_action_just_pressed("disparos_magicos") \
	and unlocked_powers["disparos_magicos"] \
	and power_ready["disparos_magicos"]:
		start_cast("disparos_magicos", "cast_disparo")
		return
	
	if Input.is_action_just_pressed("rajada_gelada") \
	and unlocked_powers["rajada_gelada"] \
	and power_ready["rajada_gelada"]:
		start_cast("rajada_gelada", "cast_rajada")
		return
	
	if Input.is_action_just_pressed("bola_de_fogo") \
	and unlocked_powers["bola_de_fogo"] \
	and power_ready["bola_de_fogo"]:
		start_cast("bola_de_fogo", "cast_fogo")
		return
	
	if Input.is_action_just_pressed("poder_fenomenal") \
	and unlocked_powers["poder_fenomenal"] \
	and power_ready["poder_fenomenal"]:
		start_cast("poder_fenomenal", "cast_fenomenal")
		return

func start_cast(spell_name: String, animation_name: String):
	is_casting = true
	pending_spell = spell_name
	$AnimatedSprite2D.play(animation_name)

func update_animation():
	if is_casting:
		return
	
	var direction := Input.get_axis("left", "right")
	$AnimatedSprite2D.trigger_animation(velocity, direction)

func spawn_disparos_magicos():
	var spell_instance = DISPARO.instantiate()
	spell_instance.global_position = $Spell_point.global_position
	spell_instance.setup(facing_direction, 2, "", 0.0, self)
	get_parent().add_child(spell_instance)

func spawn_rajada_gelada():
	var spell_instance = RAJADA.instantiate()
	spell_instance.global_position = $Spell_point2.global_position
	spell_instance.setup(facing_direction, 10, "freeze", 5.0, self)
	get_parent().add_child(spell_instance)

func spawn_bola_de_fogo():
	var spell_instance = FOGO.instantiate()
	spell_instance.global_position = $Spell_point.global_position
	spell_instance.setup(facing_direction, 15, "burn", 4.0, self)
	get_parent().add_child(spell_instance)

func spawn_poder_fenomenal():
	var spell_instance = FENOMENAL.instantiate()
	spell_instance.global_position = $Spell_point.global_position
	spell_instance.setup(facing_direction, 25, "mark", 6.0, self)
	get_parent().add_child(spell_instance)

func finish_cast():
	match pending_spell:
		"disparos_magicos":
			spawn_disparos_magicos()
		"rajada_gelada":
			spawn_rajada_gelada()
		"bola_de_fogo":
			spawn_bola_de_fogo()
		"poder_fenomenal":
			spawn_poder_fenomenal()
	
	start_power_cooldown(pending_spell)
	
	pending_spell = ""
	is_casting = false

func start_power_cooldown(power_name: String):
	if power_name == "":
		return
	
	power_ready[power_name] = false
	power_cooldown_remaining[power_name] = power_cooldowns[power_name]
	print(power_name, " entrou em cooldown por ", power_cooldowns[power_name], " segundos")

func _on_animated_sprite_2d_animation_finished() -> void:
	var anim = $AnimatedSprite2D.animation
	
	if anim == "cast_disparo" \
	or anim == "cast_rajada" \
	or anim == "cast_fogo" \
	or anim == "cast_fenomenal":
		finish_cast()

func _on_freeze_timer_timeout() -> void:
	if is_dead:
		return
	
	is_frozen = false
	print("Player descongelou.")

func _on_burn_timer_timeout() -> void:
	if is_dead:
		return
	
	is_burning = false
	$BurnTickTimer.stop()
	print("Queimadura terminou")

func _on_burn_tick_timer_timeout() -> void:
	if is_dead:
		return
	
	if is_burning:
		take_damage(burn_damage)

func _on_mark_timer_timeout() -> void:
	if is_dead:
		return
	
	is_marked = false
	print("Marca expirou")
